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

#include "r_common.h"
#include "./Entity/base_game_entity.h"
#include "map_parser.h"
#include "polysoup.h"

class CWorld {
public:
    void InitWorld(glm::vec3 gravity);
    void InitWorldFromMap(const Map& map);
    void InitStaticGeometry(std::vector<MapTri> tris);
    void AddDynamicGeometry(std::vector<MapTri> tris);

    static std::vector<MapTri> CWorld::CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys);
    
    std::vector<MapTri>          m_MapTris;
    uint64_t                     m_StaticGeometryEndIndex;
    bool                         m_StaticGeometryInitialized = false;
    std::vector<int>             m_BrushEntities;
    glm::vec3                    m_Gravity;

};



#endif // _CWORLD_H_

