//
// Created by me on 9/1/24.
//

#include "CWorld.h"

#include <assert.h>

#include <string.h>
#include <unordered_set>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "Audio/Audio.h"
#include "Entity/FirstPersonPlayer/g_fp_player.h"
#include "Entity/base_game_entity.h"
#include "Message/g_extra_info_types.h"
#include "Message/message_type.h"
#include "hkd_interface.h"
#include "irender.h"
#include "map_parser.h"
#include "platform.h"
#include "polysoup.h"
#include "r_draw.h"
#include "utils/quick_math.h"
#include "utils/utils.h"

extern std::string g_GameDir;

CWorld* CWorld::Instance()
{
    static CWorld m_World;

    return &m_World;
}

void CWorld::InitWorld(const std::string& mapName)
{
    // Get some subsystems
    EntityManager* m_pEntityManager = EntityManager::Instance();
    IRender*       renderer         = GetRenderer();

    std::string exePath = hkd_GetExePath();

    std::string mapData     = loadTextFile(g_GameDir + "maps/" + mapName + ".map");
    MapVersion  mapVersion  = VALVE_220;
    size_t      inputLength = mapData.length();
    Map         map         = getMap(&mapData[ 0 ], inputLength, mapVersion);

    // TODO: Init via .MAP property.
    m_Gravity = glm::vec3(0.0f, 0.0f, -200.0f);

    // Convert to tris

    // Check if a lightmap is available
    m_LightmapAvailable = false;
    HKD_File    plyFile;
    std::string fullPlyPath = g_GameDir + "maps/" + mapName + ".ply";
    if ( hkd_read_file(fullPlyPath.c_str(), &plyFile) == HKD_FILE_SUCCESS )
    {
        m_MapTris           = CWorld::CreateMapFromLightmapTrisFile(plyFile);
        m_hLightmapTexture  = renderer->RegisterTextureGetHandle(mapName + ".png");
        m_LightmapAvailable = true;
    }
    else
    {
        printf("Warning (CWorld): Lightmap file '%s' could not be loaded!\n", mapName.c_str());
    }

    // No lightmap provided or found. It's fine. The world can be initialized
    // from the ASCII MAP file as well. But everything will be bright.
    if ( !m_LightmapAvailable )
    {
        // Get static geometry from map
        std::vector<MapPolygon> polysoup = createPolysoup(map);
        m_MapTris                        = CWorld::CreateMapTrisFromMapPolys(polysoup);
    }

    m_StaticGeometryCount = m_MapTris.size();

    // Now initialize all the entities. Those also include brush
    // entities. Those entities store their own geometry as MapTris.
    // We keep pointers to those triangles as the brush entities (eg. doors)
    // update their position. We must know about this positional update
    // in order to collide with the tris correctly.

    // Load and create all the entities
    for ( int i = 0; i < map.entities.size(); i++ )
    {
        const Entity& e = map.entities[ i ];
        // Check the classname
        for ( int j = 0; j < e.properties.size(); j++ )
        {
            const Property& prop = e.properties[ j ];
            if ( prop.key == "classname" )
            {
                if ( prop.value == "func_door" )
                {
                    Door* door = new Door(e.properties, e.brushes);
                    m_pEntityManager->RegisterEntity(door);
                    HKD_Model* model = door->GetModel();
                    m_pBrushModels.push_back(model);
                    std::vector<MapTri>& mapTris = door->MapTris();
                    m_pBrushMapTris.push_back(&mapTris);
                }
                else if ( prop.value == "info_player_start" )
                {
                    assert(m_pPlayerEntity == nullptr); // There can only be one
                    m_pPlayerEntity = new FirstPersonPlayer(e.properties);
                    m_pEntityManager->RegisterEntity(m_pPlayerEntity);
                    // Upload this model to the GPU. Not using the handle atm.
                    int hPlayerModel = renderer->RegisterModel(m_pPlayerEntity->GetModel());
                    m_pModels.push_back(m_pPlayerEntity->GetModel());
                }
                else if ( prop.value == "monster_soldier" )
                {
                    // just a placeholder entity from trenchbroom/quake
                    Enemy* enemy = new Enemy(e.properties);
                    m_pEntityManager->RegisterEntity(enemy);
                    int hEnemyModel = renderer->RegisterModel(enemy->GetModel());
                    m_pModels.push_back(enemy->GetModel());
                }
                else if ( prop.value == "path_corner" )
                { // FIX: Should be an entity type as well.
                    Waypoint point = CWorld::GetWaypoint(&e);
                    m_NameToWaypoint.insert({ point.targetname, point });
                }
                else
                {
                    printf("Unknown entity type: %s\n", prop.value.c_str());
                }
            }
        }
    }

    // NOTE: At the moment, maps without an 'info_player_start' are not allowed.
    // FIX: (Michael): I think it should be possible to not have a player?
    assert(m_pPlayerEntity != nullptr);

    // Create paths from waypoints
    std::unordered_set<std::string> visited; // waypoints that are already part of any path

    for ( const auto& [ targetname, waypoint ] : m_NameToWaypoint )
    {
        // Skip waypoints already in a patrol path
        if ( visited.count(targetname) )
        {
            continue;
        }

        // Start a new patrol path
        PatrolPath                      currentPath;
        Waypoint                        current = waypoint;
        std::unordered_set<std::string> pathVisited; // Track for cycles

        while ( !current.target.empty() )
        { // Does the waypoint point to another?
            if ( pathVisited.count(current.targetname) )
            {
                // Cycle detected, break the path
                break;
            }

            pathVisited.insert(current.targetname);
            currentPath.AddPoint(current);
            visited.insert(current.targetname);

            // Move to the next waypoint if it exists
            auto it = m_NameToWaypoint.find(current.target);
            if ( it != m_NameToWaypoint.end() )
            {
                current = it->second;
            }
            else
            {
                break; // End of the chain
            }
        }

        // Add the completed path to the list of paths
        if ( !currentPath.GetPoints().empty() )
        {
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
    for ( int i = 0; i < entities.size(); i++ )
    {
        BaseGameEntity* entity = entities[ i ];
        if ( entity->Type() == ET_ENEMY )
        {
            Enemy* enemy = (Enemy*)entity;
            if ( !enemy->m_Target.empty() )
            {
                // Find the waypoint and its path
                // FIX: This also is easier if waypoints are entities
                for ( int j = 0; j < m_Paths.size(); j++ )
                {
                    PatrolPath&           path   = m_Paths[ j ];
                    std::vector<Waypoint> points = path.GetPoints();
                    for ( int k = 0; k < points.size(); k++ )
                    {
                        Waypoint point = points[ k ];
                        if ( enemy->m_Target == point.targetname )
                        {
                            // copy its path for internal use in this enemy
                            PatrolPath* pPathCopy = new PatrolPath(&path);
                            pPathCopy->SetCurrentWaypoint(enemy->m_Target);
                            pPathCopy->SetNextWaypoint(point.target);
                            enemy->SetPatrolPath(pPathCopy);
                            Dispatcher->DispatchMessage(
                                SEND_MSG_IMMEDIATELY, enemy->ID(), enemy->ID(), message_type::SetPatrol, 0);
                        }
                    }
                }
            }
        }
    }

    m_MusicIdle       = Audio::LoadSource("music/GranVals_Placeholder.wav", 0.0f, true, true);
    m_MusicIdleHandle = Audio::m_MusicBus.play(*m_MusicIdle, -1);

    m_Ambience = Audio::LoadSource(
        "ambience/sonniss/DSGNDron_EMF_Designed_Drone_Ambience_Forbidden_Cave.wav", 0.15f, true, true);
    m_AmbienceHandle = Audio::m_AmbienceBus.play(*m_Ambience, -1);
}

void CWorld::CollideEntitiesWithWorld()
{
    std::vector<BaseGameEntity*> entities    = EntityManager::Instance()->Entities();
    double                       dt          = GetDeltaTime();
    static double                accumulator = 0.0;
    accumulator += dt;
    int numUpdateSteps = 0;

    while ( accumulator >= DOD_FIXED_UPDATE_TIME )
    {
        for ( int i = 0; i < entities.size(); i++ )
        {
            BaseGameEntity* pEntity = entities[ i ];

            if ( (pEntity->Type() == ET_PLAYER) || (pEntity->Type() == ET_ENEMY) )
            {

                EllipsoidCollider* ec = pEntity->GetEllipsoidColliderPtr();
                if ( ec == nullptr )
                {
                    continue;
                }

                pEntity->m_PrevPosition = ec->center; //pEntity->m_Position;
                //printf("velocity: %f %f %f\n", pEntity->m_Velocity.x, pEntity->m_Velocity.y, pEntity->m_Velocity.z);
                CollisionInfo collisionInfo
                    = CollideEllipsoidWithMapTris(*ec,
                                                  (float)DOD_FIXED_UPDATE_TIME / 1000.0f * pEntity->m_Velocity,
                                                  (float)DOD_FIXED_UPDATE_TIME / 1000.0f * m_Gravity,
                                                  m_MapTris.data(),
                                                  StaticGeometryCount(),
                                                  m_pBrushMapTris);

                // FIX: Again. Components could fix this.
                HKD_Model* model = nullptr;
                if ( pEntity->Type() == ET_PLAYER )
                {
                    model = ((FirstPersonPlayer*)pEntity)->GetModel();
                }
                else if ( pEntity->Type() == ET_ENEMY )
                {
                    model = ((Enemy*)pEntity)->GetModel();
                }

                // Update the position of the collider.
                for ( int i = 0; i < model->animations.size(); i++ )
                {
                    model->ellipsoidColliders[ i ].center = collisionInfo.basePos;
                }

            } // Check if player or enemy
        } // Check all entities
        accumulator -= DOD_FIXED_UPDATE_TIME;
        numUpdateSteps++;
    } // while (accumulator >= DOD_FIXED_UPDATE_TIME) {

    // Avoid 'spiral of hell'.
    if ( numUpdateSteps > 5 )
    {
        printf("WARNING: SPIRAL OF HELL!!!\n");
        accumulator = 0;
    }

    // Now, update all the entities positions. But only if they are of
    // a certain type.
    // FIX: Dumb! We should loop over the collider components and
    // then update their owners. Again: We need the actor-component model ;)
    for ( int i = 0; i < entities.size(); i++ )
    {
        BaseGameEntity* pEntity = entities[ i ];
        if ( (pEntity->Type() == ET_PLAYER) || (pEntity->Type() == ET_ENEMY) )
        {
            EllipsoidCollider* ec = pEntity->GetEllipsoidColliderPtr();

            if ( ec == nullptr )
            {
                continue;
            }

            double    t             = accumulator / DOD_FIXED_UPDATE_TIME;
            glm::vec3 perTickMotion = ec->center - pEntity->m_PrevPosition;
            pEntity->UpdatePosition(pEntity->m_PrevPosition + (float)t * perTickMotion);

            // Check if entity is in air.
            CollisionInfo ci = PushTouch(*ec, -DOD_WORLD_UP * 7.0f, m_MapTris.data(), StaticGeometryCount());

            if ( ci.didCollide )
            {
                pEntity->m_CollisionState = ES_ON_GROUND;
            }
            else
            {
                pEntity->m_CollisionState = ES_IN_AIR;
            }

            // Also check for brush entities such as doors
            // (we could stand on top of them).
            for ( int i = 0; i < m_pBrushMapTris.size(); i++ )
            {
                std::vector<MapTri>* brushMapTris = m_pBrushMapTris[ i ];

                CollisionInfo brushCi
                    = PushTouch(*ec, -DOD_WORLD_UP * 7.0f, brushMapTris->data(), brushMapTris->size());
                if ( brushCi.didCollide )
                {
                    pEntity->m_CollisionState = ES_ON_GROUND;
                    break;
                }
            }
        }
    }
}

// FIX: Slow. Can we do better? Push touch is more than an overlap check!
// We need to recheck all the entities in the inner loop because only
// the first entity from the outer loop is the one who 'pushes'.
void CWorld::CollideEntities()
{

    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    double                       dt       = GetDeltaTime();

    // Push Touch: Entity 'bumps' into other entity.
    for ( int i = 0; i < entities.size(); i++ )
    {
        BaseGameEntity* pEntity = entities[ i ];

        // Ignore non moving entities. They cannot 'bump' without velocity.
        if ( glm::length(pEntity->m_Velocity) <= 0.0f )
        {
            continue;
        }

        for ( int j = 0; j < entities.size(); j++ )
        {

            BaseGameEntity* pOther = entities[ j ];

            if ( pOther->ID() == pEntity->ID() )
            { // Don't collide with itself.
                continue;
            }

            EllipsoidCollider* ec = pEntity->GetEllipsoidColliderPtr();
            if ( ec == nullptr ) // we nned an ellipsoid collider to collide with something
            {
                continue;
            }

            CollisionInfo ci{};

            if ( pOther->Type() == ET_DOOR )
            {
                Door* pDoor = (Door*)pOther;

                ci = PushTouch(*ec,
                               (float)DOD_FIXED_UPDATE_TIME / 1000.0f * pEntity->m_Velocity,
                               pDoor->MapTris().data(),
                               pDoor->MapTris().size());
            }
            else
            {
            }

            if ( ci.didCollide )
            {
                printf("COLLIDED!\n");
                Dispatcher->DispatchMessage(
                    SEND_MSG_IMMEDIATELY, pEntity->ID(), pOther->ID(), message_type::Collision, 0);
            }
        }
    }
}

void CWorld::RunEnemyVision()
{
    std::vector<BaseGameEntity*> entities = EntityManager::Instance()->Entities();
    double                       dt       = GetDeltaTime();

    // Push Touch: Entity 'bumps' into other entity.
    for ( int i = 0; i < entities.size(); i++ )
    {
        BaseGameEntity* pEntity = entities[ i ];

        if ( pEntity->Type() == ET_ENEMY )
        {
            Enemy* pEnemy = (Enemy*)pEntity;

            for ( int j = 0; j < entities.size(); j++ )
            {

                BaseGameEntity* pOther = entities[ j ];
                if ( pOther->ID() == pEntity->ID() )
                { // Don't look at ourselves
                    continue;
                }

                EllipsoidCollider* ec = pOther->GetEllipsoidColliderPtr();
                if ( ec == nullptr )
                {
                    continue;
                }

                glm::vec3 enemyFrustumPosition = pEnemy->m_Position;
                enemyFrustumPosition.z += pEnemy->GetEllipsoidColliderPtr()->radiusB;
                glm::mat4 enemyTransform = glm::translate(glm::mat4(1.0f), enemyFrustumPosition);
                glm::mat4 enemyRotation  = glm::toMat4(pEnemy->m_Orientation);
                enemyTransform           = enemyTransform * enemyRotation;

                math::Frustum frustumWorld = math::BuildFrustum(
                    enemyTransform, pEnemy->m_ProjDistance, pEnemy->m_AspectRatio, pEnemy->m_Near, pEnemy->m_Far);

                r_DrawFrustum(frustumWorld);

                if ( math::EllipsoidInFrustum(frustumWorld, *ec) )
                {
                    InViewInfo inViewInfo = { pOther };
                    Dispatcher->DispatchMessage(
                        SEND_MSG_IMMEDIATELY, pEnemy->ID(), pEnemy->ID(), message_type::EntityInView, &inViewInfo);
                }
            }
        }
    }
}

std::vector<MapTri> CWorld::CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys)
{
    // Get the renderer to register the textures
    IRender* renderer = GetRenderer();

    std::vector<MapTri>     mapTris{};
    std::vector<MapPolygon> tris     = triangulate(mapPolys);
    glm::vec4               triColor = glm::vec4(0.1f, 0.8f, 1.0f, 1.0f);
    for ( int i = 0; i < tris.size(); i++ )
    {
        MapPolygon mapPoly = tris[ i ];

        Vertex A = { glm::vec3(mapPoly.vertices[ 0 ].pos.x, mapPoly.vertices[ 0 ].pos.y, mapPoly.vertices[ 0 ].pos.z),
                     mapPoly.vertices[ 0 ].uv };
        Vertex B = { glm::vec3(mapPoly.vertices[ 1 ].pos.x, mapPoly.vertices[ 1 ].pos.y, mapPoly.vertices[ 1 ].pos.z),
                     mapPoly.vertices[ 1 ].uv };
        Vertex C = { glm::vec3(mapPoly.vertices[ 2 ].pos.x, mapPoly.vertices[ 2 ].pos.y, mapPoly.vertices[ 2 ].pos.z),
                     mapPoly.vertices[ 2 ].uv };

        A.color         = triColor;
        B.color         = triColor;
        C.color         = triColor;
        MapTri tri      = { .tri = { A, B, C } };
        tri.textureName = mapPoly.textureName;
        //FIX: Search through all supported image formats not just PNG.
        tri.hTexture = renderer->RegisterTextureGetHandle(tri.textureName + ".tga");
        mapTris.push_back(tri);
    }

    return mapTris;
}

// NOTE: Keep this. Getting properties is done via the template stuff
// in base_entity but maybe the template stuff turns out to be dumb.
glm::vec3 CWorld::GetOrigin(const Entity* entity)
{
    for ( const Property& property : entity->properties )
    {
        if ( property.key == "origin" )
        {
            std::vector<float> values = ParseValues<float>(property.value);
            return glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        }
    }

    assert(false && "Entity has no origin property!");
}

// NOTE: Keep this. Getting properties is done via the template stuff
// in base_entity but maybe the template stuff turns out to be dumb.
Waypoint CWorld::GetWaypoint(const Entity* entity)
{
    Waypoint waypoint = {};
    for ( const Property& property : entity->properties )
    {
        if ( property.key == "origin" )
        {
            std::vector<float> values = ParseValues<float>(property.value);
            waypoint.position         = glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
        }
        else if ( property.key == "targetname" )
        {
            waypoint.targetname = property.value;
        }
        else if ( property.key == "target" )
        {
            waypoint.target = property.value;
        }
    }

    return waypoint;
}

Vertex CWorld::StaticVertexToVertex(StaticVertex staticVertex)
{
    Vertex result{ staticVertex.pos, staticVertex.uv, staticVertex.uvLightmap, staticVertex.bc, staticVertex.normal };

    return result;
}

std::vector<MapTri> CWorld::CreateMapFromLightmapTrisFile(HKD_File lightmapTrisFile)
{
    /*
struct MapTriLightmapper {
    StaticVertex vertices[ 3 ];
    char         textureName[ 64 ];
    uint64_t     surfaceFlags;
    uint64_t     contentFlags;
};

struct MapTri {
    Tri         tri;
    std::string textureName;
    std::string lightmap;
    uint64_t    hTexture; // GPU handle set by renderer.
};
*/
    // Get the renderer to register the textures
    IRender* renderer = GetRenderer();

    MapTriLightmapper*             currentLightmapTri;
    std::vector<MapTriLightmapper> lightmapperTris{};
    std::vector<MapTri>            mapTris{};

    size_t numLightmapTris = lightmapTrisFile.size / sizeof(MapTriLightmapper);
    mapTris.resize(numLightmapTris);

    currentLightmapTri = (MapTriLightmapper*)lightmapTrisFile.data;
    for ( int i = 0; i < numLightmapTris; i++ )
    {
        MapTri mapTri{};
        Vertex vertices[ 3 ];
        vertices[ 0 ] = StaticVertexToVertex(currentLightmapTri->vertices[ 0 ]);
        vertices[ 1 ] = StaticVertexToVertex(currentLightmapTri->vertices[ 1 ]);
        vertices[ 2 ] = StaticVertexToVertex(currentLightmapTri->vertices[ 2 ]);

        memcpy(mapTri.tri.vertices, vertices, 3 * sizeof(Vertex));

        mapTri.textureName = std::string(currentLightmapTri->textureName);
        mapTri.hTexture    = renderer->RegisterTextureGetHandle(mapTri.textureName + ".tga");

        mapTris[ i ] = mapTri;

        currentLightmapTri++;
    }

    return mapTris;
}
