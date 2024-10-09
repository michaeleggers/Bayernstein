//
// Created by me on 9/1/24.
//

#ifndef CWORLD_H
#define CWORLD_H

#include "r_common.h"

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include <vector>
#include <stdint.h>

class CWorld {
public:
    void InitWorld(TriPlane* triPlanes, uint32_t triPlaneCount, glm::vec3 gravity);

    std::vector<TriPlane> m_TriPlanes;
	glm::vec3			  m_Gravity;
};



#endif //CWORLD_H
