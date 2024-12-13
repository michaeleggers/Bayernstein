//
// Created by me on 9/1/24.
//

#ifndef _CWORLD_H_
#define _CWORLD_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "./Entity/base_game_entity.h"
#include "Entity/Door/g_door.h"
#include "Entity/Enemy/g_enemy.h"
#include "Entity/FirstPersonPlayer/g_fp_player.h"
#include "Entity/FlyCamera/g_fly_camera.h"
#include "Entity/FollowCamera/g_follow_camera.h"
#include "Entity/Player/g_player.h"
#include "Entity/entity_manager.h"
#include "Path/path.h"
#include "map_parser.h"
#include "platform.h"
#include "polysoup.h"
#include "r_common.h"
#include "r_model.h"

class CWorld {
  public:
    static CWorld* Instance();
    void           InitWorldFromMap(const Map& map, const std::string& plyFilename);
    void           CollideEntitiesWithWorld();
    void           CollideEntities();

    /*
    * Easier to understand in code when only the number of
    * static tris is needed.
    */
    uint64_t StaticGeometryCount() {
        return m_StaticGeometryCount;
    }

    FirstPersonPlayer* PlayerEntity() {
        return m_pPlayerEntity;
    }

    std::vector<MapTri>& GetMapTris() {
        return m_MapTris;
    }

    std::vector<HKD_Model*>& GetModelPtrs() {
        return m_pModels;
    }

    std::vector<HKD_Model*>& GetBrushModelPtrs() {
        return m_pBrushModels;
    }

    static glm::vec3           GetOrigin(const Entity* entity);
    static Waypoint            GetWaypoint(const Entity* entity);
    static std::vector<MapTri> CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys);
    static std::vector<MapTri> CreateMapFromLightmapTrisFile(HKD_File lightmapTrisFile);
    static Vertex              StaticVertexToVertex(StaticVertex staticVertex);

    std::vector<MapTri> m_MapTris;
    glm::vec3           m_Gravity;
    // FIX: Where to put paths? Shouldn't they also be entities themselves??
    std::unordered_map<std::string, Waypoint> m_NameToWaypoint;
    std::vector<PatrolPath>                   m_Paths;

  private:
    CWorld()  = default;
    ~CWorld() = default;

    uint64_t m_StaticGeometryCount;
    // FIX: Does the player really *always* have to exist?
    FirstPersonPlayer* m_pPlayerEntity = nullptr;

    // Model entities that are defined via an IQM model.
    std::vector<HKD_Model*> m_pModels;

    // Models for brush entities.
    std::vector<HKD_Model*> m_pBrushModels;

    // Keep references to brush entities' map tris so we
    // can easily collide against brush entities as well.
    std::vector<std::vector<MapTri>*> m_pBrushMapTris;

    // If a lightmap is loaded, this handle stores
    // the texture handle on the GPU.
    uint64_t m_hLightmapTexture;
};

#endif // _CWORLD_H_
