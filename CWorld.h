//
// Created by me on 9/1/24.
//

#ifndef _CWORLD_H_
#define _CWORLD_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include <vector>
#include <stdint.h>
#include <unordered_map>

#include "r_common.h"
#include "./Entity/base_game_entity.h"
#include "Entity/Door/g_door.h"
#include "Entity/Enemy/g_enemy.h"
#include "Entity/Player/g_player.h"
#include "Entity/FollowCamera/g_follow_camera.h"
#include "Entity/FlyCamera/g_fly_camera.h"
#include "Entity/entity_manager.h"
#include "Path/path.h"
#include "map_parser.h"
#include "polysoup.h"
#include "r_model.h"

class CWorld {
public:
    static CWorld* Instance();
    void        InitWorld(glm::vec3 gravity);
    void        InitWorldFromMap(const Map& map);
    void        InitStaticGeometry(std::vector<MapTri> tris);
    void        AddBrushesToDynamicGeometry(const std::vector<Brush>& brushes);
    void        CollideEntitiesWithWorld();
    void        CollideEntities();
    
    uint64_t OffsetDynamicGeometry() {
        return m_OffsetDynamicGeometry;
    }
   
    /*
    * Easier to understand in code when only the number of
    * static tris is needed.
    */
    uint64_t StaticGeometryCount() {
        return m_OffsetDynamicGeometry;
    }

    Player* PlayerEntity() {
        return m_pPlayerEntity;
    }

    std::vector<MapTri>& MapTris() {
        return m_MapTris;
    }

    std::vector<HKD_Model*>& Models() {
        return m_Models;
    }

    std::vector<HKD_Model*>& BrushModels() {
        return m_BrushModels;
    }

    static glm::vec3            GetOrigin(const Entity* entity);
    static Waypoint             GetWaypoint(const Entity* entity);
    static std::vector<MapTri>  CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys);
    
    std::vector<MapTri>          m_MapTris;
    uint64_t                     m_StaticGeometryEndIndex;
    bool                         m_StaticGeometryInitialized = false;
    glm::vec3                    m_Gravity;
    // FIX: Where to put paths? Shouldn't they also be entities themselves??
    std::unordered_map<std::string, Waypoint> m_NameToWaypoint;
    PatrolPath*                  m_pPath = nullptr;

private:
    CWorld() = default;
    ~CWorld() = default;

    uint64_t                     m_OffsetDynamicGeometry;
    // FIX: Does the player really *always* have to exist?
    Player*                      m_pPlayerEntity = nullptr;
    std::vector<HKD_Model*>      m_Models; 
    std::vector<HKD_Model*>      m_BrushModels;
    // Keep references to brush entities' map tris
    std::vector< std::vector<MapTri>* >  m_pBrushMapTris;
};



#endif // _CWORLD_H_

