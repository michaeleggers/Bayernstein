//
// Created by me on 9/1/24.
//

#include "CWorld.h"
#include <string.h>
#include <vector>

void CWorld::InitWorld(std::vector<MapTri> tris, glm::vec3 gravity) {
    m_MapTris = tris;
    m_Gravity = gravity;
}

