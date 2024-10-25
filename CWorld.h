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


class CWorld {
public:
    void InitWorld(TriPlane* triPlanes, uint32_t triPlaneCount, glm::vec3 gravity);

    std::vector<TriPlane>        m_TriPlanes;
    std::vector<int>             m_BrushEntities;
    glm::vec3                    m_Gravity;

};



#endif // _CWORLD_H_

