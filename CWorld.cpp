//
// Created by me on 9/1/24.
//

#include "CWorld.h"

#include <assert.h>

#include <string.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "map_parser.h"
#include "irender.h"
#include "hkd_interface.h"
#include "Entity/base_game_entity.h"
#include "utils/utils.h"
#include "Message/message_type.h"

void CWorld::InitWorld(glm::vec3 gravity) {
    m_Gravity = gravity;
}

void CWorld::InitWorldFromMap(const Map& map) {
    // Get some subsystems
    EntityManager* m_pEntityManager = EntityManager::Instance();
    IRender* renderer = GetRenderer();
   
    // FIX: See declaration in header.
    m_pPath = new PatrolPath();
   
    // TODO: Init via .MAP property.
    m_Gravity = glm::vec3(0.0f, 0.0f, -0.5f);

    // Get static geometry from map
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    // Convert to tris
    m_MapTris = CWorld::CreateMapTrisFromMapPolys(polysoup);
    
    // Everything *after* the static geometry is dynamic (brush entities).
    // Remember where they start:
    m_OffsetDynamicGeometry = m_MapTris.size();

    // Now initialize all the entities. Those also include brush
    // entities that get appended to m_MapTris. We remember where
    // the brush entities start in m_OffsetDynamicGeometry.
    // Load and create all the entities
    for ( int i = 0; i < map.entities.size(); i++ ) {
        const Entity& e = map.entities[ i ];
        BaseGameEntity* baseEntity = NULL;
        // Check the classname
        for ( int j = 0; j < e.properties.size(); j++ ) {
            const Property& prop = e.properties[ j ];
            if ( prop.key == "classname" ) {
                if ( prop.value == "func_door" ) {
                    baseEntity = new Door(e.properties, e.brushes); 
                    AddBrushesToDynamicGeometry( e.brushes );
                    m_BrushEntities.push_back(baseEntity->ID());
                    m_pEntityManager->RegisterEntity(baseEntity); 
                    HKD_Model* model = ((Door*)baseEntity)->GetModel();
                    m_BrushModels.push_back( model );
                } else if ( prop.value == "info_player_start" ) {
                    assert( m_pPlayerEntity == nullptr ); // There can only be one
                    glm::vec3 playerStartPosition = CWorld::GetOrigin(&e);
                    m_pPlayerEntity = new Player(playerStartPosition);
                    m_pEntityManager->RegisterEntity(m_pPlayerEntity);
                    // Upload this model to the GPU. Not using the handle atm.
                    int hPlayerModel = renderer->RegisterModel(m_pPlayerEntity->GetModel());
                    m_Models.push_back(m_pPlayerEntity->GetModel());
                } else if ( prop.value == "monster_soldier" ) {
                    // just a placeholder entity from trenchbroom/quake
                    glm::vec3 enemyStartPosition = CWorld::GetOrigin(&e);
                    Enemy* enemy = new Enemy(enemyStartPosition);
                    m_pEntityManager->RegisterEntity(enemy);

                    int hEnemyModel = renderer->RegisterModel(enemy->GetModel());
                    m_Models.push_back(enemy->GetModel());
                } else if ( prop.value == "path_corner" ) { // FIX: Should be an entity type as well.
                    Waypoint point = CWorld::GetWaypoint(&e);

                    // I assume that the corner Points are in the right order. if not we need to rethink the data structure
                    glm::vec3 pathCornerPosition = CWorld::GetOrigin(&e);
                    m_pPath->AddPoint(point);
                    printf("Path corner entity found: %f, %f, %f\n",
                           pathCornerPosition.x,
                           pathCornerPosition.y,
                           pathCornerPosition.z);
                } else {
                    printf("Unknown entity type: %s\n", prop.value.c_str());
                }
            }
        }
    }

    // NOTE: At the moment, maps without an 'info_player_start' are not allowed.
    assert( m_pPlayerEntity != nullptr );

    // FIX: The path should be set by the level designer in TrenchBroom. Just for debug now.
    Enemy* enemy = m_pEntityManager->GetFirstEnemy();
    // enemy->SetArriveTarget(m_pPlayerEntity);
    enemy->SetFollowPath(m_pPath);
}

void CWorld::InitStaticGeometry(std::vector<MapTri> tris) {
    // Static Geometry cannot be initialized twice!
    assert( !m_StaticGeometryInitialized );

    m_MapTris = tris;

    m_StaticGeometryInitialized = true;
}

void CWorld::AddBrushesToDynamicGeometry(const std::vector<Brush>& brushes) {
    for (int i = 0; i < brushes.size(); i++) {
        std::vector<MapPolygon> mapPolys = createPolysoup( brushes[i] );
        std::vector<MapTri> mapTris = CreateMapTrisFromMapPolys(mapPolys);
        std::copy( mapTris.begin(), mapTris.end(), std::back_inserter(m_MapTris) );
    }
}

void CWorld::CollideEntitiesWithWorld() {
    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    double dt = GetDeltaTime();
    for (int i = 0; i < entities.size(); i++) {
        BaseGameEntity* pEntity = entities[i];

        // FIX: Higher level entity type or entity flags (eg. FLAG_COLLIDABLE).
        if ( (pEntity->Type() == ET_PLAYER) || (pEntity->Type() == ET_ENEMY) ) {

            EllipsoidCollider ec = pEntity->GetEllipsoidCollider();
            //printf("velocity: %f %f %f\n", pEntity->m_Velocity.x, pEntity->m_Velocity.y, pEntity->m_Velocity.z);
            CollisionInfo collisionInfo = CollideEllipsoidWithMapTris(ec,
                                                                      static_cast<float>(dt) * pEntity->m_Velocity,
                                                                      static_cast<float>(dt) * m_Gravity,
                                                                      m_MapTris.data(),
                                                                      m_MapTris.size());

            pEntity->UpdatePosition(collisionInfo.basePos);
        }
    }
}

// FIX: Slow. Can we do better? Push touch is more than an overlap check!
// We need to recheck all the entities in the inner loop because only
// the first entity from the outer loop is the one who 'pushes'.
void CWorld::CollideEntities() {
    
    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    double dt = GetDeltaTime();
  
    // Push Touch: Entity 'bumps' into other entity.
    for (int i = 0; i < entities.size(); i++) {
        BaseGameEntity* pEntity = entities[i];
        
        // Ignore non moving entities. They cannot 'bump' without velocity.
        if ( glm::length(pEntity->m_Velocity) <= 0.0f ) {
            continue;
        }

        for (int j = 0; j < entities.size(); j++) {
  
            BaseGameEntity* pOther = entities[j];
            if ( pOther->ID() == pEntity->ID() ) { // Don't collide with itself.
                continue;
            }
   
            if ( pOther->Type() == ET_DOOR ) {
                Door* pDoor = (Door*)pOther;
                EllipsoidCollider ec = pEntity->GetEllipsoidCollider();
                CollisionInfo ci = PushTouch(ec,
                                             static_cast<float>(dt) * pEntity->m_Velocity, 
                                             pDoor->MapTris().data(), 
                                             pDoor->MapTris().size());
                if (ci.didCollide) { 
                    printf("COLLIDED!\n");
                    Dispatcher->DispatchMessage(
                        SEND_MSG_IMMEDIATELY, pEntity->ID(), pDoor->ID(), message_type::Collision, 0);
                }
            }
            
        }
    }
}

std::vector<MapTri> CWorld::CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys) {
    // Get the renderer to register the textures
    IRender* renderer = GetRenderer();

    std::vector<MapTri> mapTris{};
    std::vector<MapPolygon> tris = triangulate(mapPolys);
    glm::vec4 triColor = glm::vec4(0.1f, 0.8f, 1.0f, 1.0f);
    for ( int i = 0; i < tris.size(); i++ ) {
        MapPolygon mapPoly = tris[ i ];

        Vertex A = { glm::vec3(mapPoly.vertices[ 0 ].pos.x, mapPoly.vertices[ 0 ].pos.y, mapPoly.vertices[ 0 ].pos.z),
                     mapPoly.vertices[ 0 ].uv };
        Vertex B = { glm::vec3(mapPoly.vertices[ 1 ].pos.x, mapPoly.vertices[ 1 ].pos.y, mapPoly.vertices[ 1 ].pos.z),
                     mapPoly.vertices[ 1 ].uv };
        Vertex C = { glm::vec3(mapPoly.vertices[ 2 ].pos.x, mapPoly.vertices[ 2 ].pos.y, mapPoly.vertices[ 2 ].pos.z),
                     mapPoly.vertices[ 2 ].uv };

        A.color = triColor;
        B.color = triColor;
        C.color = triColor;
        MapTri tri = { .tri = { A, B, C } };
        tri.textureName = mapPoly.textureName;
        //FIX: Search through all supported image formats not just PNG.
        tri.hTexture = renderer->RegisterTextureGetHandle(tri.textureName + ".tga");
        mapTris.push_back(tri);
    }

    return mapTris;
}

glm::vec3 CWorld::GetOrigin(const Entity* entity) {
    for ( const Property& property : entity->properties ) {
        if ( property.key == "origin" ) {
            std::vector<float> values = ParseFloatValues(property.value);
            return glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        }
    }

    assert(false && "Entity has no origin property!");
}

Waypoint CWorld::GetWaypoint(const Entity* entity) {
    Waypoint waypoint = {};
    for ( const Property& property : entity->properties ) {
        if ( property.key == "origin" ) {
            std::vector<float> values = ParseFloatValues(property.value);
            waypoint.position = glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        } else if ( property.key == "targetname" ) {
            waypoint.sTargetname = property.value;

        } else if ( property.key == "target" ) {
            waypoint.sTarget = property.value;
        }
    }

    return waypoint;
}


