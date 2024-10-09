//
// Created by me on 9/1/24.
//

#include "CWorld.h"
#include <string.h>

void CWorld::InitWorld(TriPlane *triPlanes, uint32_t triPlaneCount, glm::vec3 gravity) {
    m_TriPlanes.resize(triPlaneCount);
    memcpy(m_TriPlanes.data(), triPlanes, triPlaneCount * sizeof(TriPlane));
	m_Gravity = gravity;
}
