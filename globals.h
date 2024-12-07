#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

constexpr glm::vec3 DOD_WORLD_UP(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 DOD_WORLD_FORWARD(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 DOD_WORLD_RIGHT(1.0f, 0.0f, 0.0f);

// Update collisions at 60Hz tickrate.
constexpr double    DOD_FIXED_UPDATE_TIME = 1000.0/60.0;
constexpr float     DOD_VERY_CLOSE_DIST   = 0.01f; // TODO: Collision with double precision?

#endif


