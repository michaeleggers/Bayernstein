//
// Created by me on 8/17/24.
//

#ifndef _INTERSECTIONS_H_
#define _INTERSECTIONS_H_

#include "Body.h"

#include <glm/glm.hpp>

struct Contact {
    glm::vec3 ptOnA_WorldSpace;
    glm::vec3 ptOnB_WorldSpace;
    glm::vec3 ptOnA_LocalSpace;
    glm::vec3 ptOnB_LocalSpace;

    glm::vec3 normal; // WorldSpace
    float     separationDistance; // + when non-penetrating, - otherwise
    float     timeOfImpact;

    Body*     bodyA;
    Body*     bodyB;
};

bool Intersect(Body* bodyA, Body* bodyB, Contact& contact);

#endif

