//
// Created by me on 9/1/24.
//

#include "CWorld.h"

#include <assert.h>

#include <string.h>
#include <vector>
#include <unordered_set>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "utils/utils.h"
#include "map_parser.h"
#include "irender.h"
#include "hkd_interface.h"
#include "Entity/base_game_entity.h"
#include "Message/message_type.h"


CWorld* CWorld::Instance() {
    static CWorld m_World;

    return &m_World;
}

void CWorld::InitWorldFromMap(const Map& map) {
    // Get some subsystems
    EntityManager* m_pEntityManager = EntityManager::Instance();
    IRender* renderer = GetRenderer();
   
    // TODO: Init via .MAP property.
    m_Gravity = glm::vec3(0.0f, 0.0f, -2.0f);

    // Get static geometry from map
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    // Convert to tris
    m_MapTris = CWorld::CreateMapTrisFromMapPolys(polysoup);
    
    m_StaticGeometryCount = m_MapTris.size();

    // Now initialize all the entities. Those also include brush
    // entities. Those entities store their own geometry as MapTris.
    // We keep pointers to those triangles as the brush entities (eg. doors)
    // update their position. We must know about this positional update
    // in order to collide with the tris correctly.

    // Load and create all the entities
    for ( int i = 0; i < map.entities.size(); i++ ) {
        const Entity& e = map.entities[ i ];
        // Check the classname
        for ( int j = 0; j < e.properties.size(); j++ ) {
            const Property& prop = e.properties[ j ];
            if ( prop.key == "classname" ) {
                if ( prop.value == "func_door" ) {
                    Door* door = new Door(e.properties, e.brushes); 
                    m_pEntityManager->RegisterEntity(door); 
                    HKD_Model* model = door->GetModel();
                    m_BrushModels.push_back( model );
                    std::vector<MapTri>& mapTris = door->MapTris();
                    m_pBrushMapTris.push_back( &mapTris );
                } else if ( prop.value == "info_player_start" ) {
                    assert( m_pPlayerEntity == nullptr ); // There can only be one
                    glm::vec3 playerStartPosition = CWorld::GetOrigin(&e);
                    m_pPlayerEntity = new FirstPersonPlayer(playerStartPosition);
                    m_pEntityManager->RegisterEntity(m_pPlayerEntity);
                    // Upload this model to the GPU. Not using the handle atm.
                    int hPlayerModel = renderer->RegisterModel(m_pPlayerEntity->GetModel());
                    m_Models.push_back(m_pPlayerEntity->GetModel());
                } else if ( prop.value == "monster_soldier" ) {
                    // just a placeholder entity from trenchbroom/quake
                    glm::vec3 enemyStartPosition = CWorld::GetOrigin(&e);
                    Enemy* enemy = new Enemy(e.properties);
                    m_pEntityManager->RegisterEntity(enemy);
                    int hEnemyModel = renderer->RegisterModel(enemy->GetModel());
                    m_Models.push_back(enemy->GetModel());
                } else if ( prop.value == "path_corner" ) { // FIX: Should be an entity type as well.
                    Waypoint point = CWorld::GetWaypoint(&e);
                    m_NameToWaypoint.insert({ point.targetname, point });
                    glm::vec3 pathCornerPosition = CWorld::GetOrigin(&e);
                } else {
                    printf("Unknown entity type: %s\n", prop.value.c_str());
                }
            }
        }
    }

    // NOTE: At the moment, maps without an 'info_player_start' are not allowed.
    // FIX: (Michael): I think it should be possible to not have a player?
    assert( m_pPlayerEntity != nullptr );

    // Create paths from waypoints
    std::unordered_set<std::string> visited; // waypoints that are already part of any path

    for (const auto& [targetname, waypoint] : m_NameToWaypoint) {
        // Skip waypoints already in a patrol path
        if (visited.count(targetname)) {
            continue;
        }

        // Start a new patrol path
        PatrolPath currentPath;
        Waypoint current = waypoint;
        std::unordered_set<std::string> pathVisited; // Track for cycles

        while ( !current.target.empty() ) { // Does the waypoint point to another?
            if (pathVisited.count(current.targetname)) {
                // Cycle detected, break the path
                break;
            }

            pathVisited.insert(current.targetname);
            currentPath.AddPoint(current);
            visited.insert(current.targetname);

            // Move to the next waypoint if it exists
            auto it = m_NameToWaypoint.find(current.target);
            if (it != m_NameToWaypoint.end()) {
                current = it->second;
            } else {
                break; // End of the chain
            }
        }

        // Add the completed path to the list of paths
        if ( !currentPath.GetPoints().empty() ) {
            m_Paths.push_back(currentPath);
        }
    }
    // Waypoints now all points to an instance inside the m_Paths vector.
    // A waypoint not being part of a 'chain' also points to a path:
    // It is a path with that poor, lonely waypoint. I kinda feel sorry for it...

    // Now that everything is initialized, set up the paths for the enemy.
    // FIX: Need a target to Entity map. But waypoints are no entities at the moment.
    // So atm it is expected that the target the entity points to is a waypoint!
    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    for (int i = 0; i < entities.size(); i++) {
        BaseGameEntity* entity = entities[i];
        if (entity->Type() == ET_ENEMY) {
            Enemy* enemy = (Enemy*)entity;
            if ( !enemy->m_Target.empty() ) {
                // Find the waypoint and its path
                // FIX: This also is easier if waypoints are entities
                for (int j = 0; j < m_Paths.size(); j++) {
                    PatrolPath& path = m_Paths[j];
                    std::vector<Waypoint> points = path.GetPoints();
                    for (int k = 0; k < points.size(); k++) {
                        Waypoint point = points[k];
                        if ( enemy->m_Target == point.targetname ) {
                            // copy its path for internal use in this enemy
                            PatrolPath* pPathCopy = new PatrolPath(&path);
                            pPathCopy->SetCurrentWaypoint(enemy->m_Target);
                            pPathCopy->SetNextWaypoint(point.target);
                            enemy->SetFollowPath(pPathCopy);
                        }
                    }
                }
            }
        }
    }
}

void CWorld::CollideEntitiesWithWorld() {
    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    double dt = GetDeltaTime();
    static const double INTERVAL = 1000.0 / 60.0;
    static double accumulator = 0.0;
    accumulator += dt;
    //printf("accumulator: %f\n", accumulator);
    for (int i = 0; i < entities.size(); i++) {
        BaseGameEntity* pEntity = entities[i];

        if ( (pEntity->Type() == ET_PLAYER) || (pEntity->Type() == ET_ENEMY) ) {

            EllipsoidCollider* ec = pEntity->GetEllipsoidColliderPtr();
            if (ec == nullptr) {
                continue;
            }

            int numUpdateSteps = 0;

            while (accumulator >= INTERVAL) {
                
                pEntity->m_PrevPosition = ec->center; //pEntity->m_Position;
                //printf("velocity: %f %f %f\n", pEntity->m_Velocity.x, pEntity->m_Velocity.y, pEntity->m_Velocity.z);
                CollisionInfo collisionInfo = CollideEllipsoidWithMapTris(*ec,
                                                                          pEntity->m_Velocity,
                                                                          m_Gravity, //glm::vec3(0.0f), //m_Gravity,
                                                                          m_MapTris.data(),
                                                                          StaticGeometryCount(),
                                                                          m_pBrushMapTris);

                //pEntity->m_Position = collisionInfo.basePos;
                ec->center = collisionInfo.basePos;
                //pEntity->UpdatePosition( pEntity->m_PrevPosition + perTickMotion );
                accumulator -= INTERVAL;

                numUpdateSteps++;
                
            } // End Update collider
            //
            
            if (numUpdateSteps > 10) {
                accumulator = 0;
            }
            
            double t = accumulator / INTERVAL;
            glm::vec3 perTickMotion = ec->center - pEntity->m_PrevPosition;
            pEntity->UpdatePosition( pEntity->m_PrevPosition + (float)t*perTickMotion );

        } // Check if Player or Enemy
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
                                             pEntity->m_Velocity, 
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

// NOTE: Keep this. Getting properties is done via the template stuff
// in base_entity but maybe the template stuff turns out to be dumb.
glm::vec3 CWorld::GetOrigin(const Entity* entity) {
    for ( const Property& property : entity->properties ) {
        if ( property.key == "origin" ) {
            std::vector<float> values = ParseValues<float>(property.value);
            return glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        }
    }

    assert(false && "Entity has no origin property!");
}

// NOTE: Keep this. Getting properties is done via the template stuff
// in base_entity but maybe the template stuff turns out to be dumb.
Waypoint CWorld::GetWaypoint(const Entity* entity) {
    Waypoint waypoint = {};
    for ( const Property& property : entity->properties ) {
        if ( property.key == "origin" ) {
            std::vector<float> values = ParseValues<float>(property.value);
            waypoint.position = glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        } else if ( property.key == "targetname" ) {
            waypoint.targetname = property.value;

        } else if ( property.key == "target" ) {
            waypoint.target = property.value;
        }
    }

    return waypoint;
}


