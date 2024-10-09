//
// Created by me on 8/17/24.
//

#include "Intersections.h"
#include "Body.h"
#include "ShapeSphere.h"

bool Intersect(Body* bodyA, Body* bodyB, Contact& contact) {
    contact.bodyA = bodyA;
    contact.bodyB = bodyB;

    glm::vec3 ab = bodyB->m_Position - bodyA->m_Position;
    contact.normal = glm::normalize(ab);
    float distanceAB = glm::length(ab);

    ShapeSphere* shapeA = (ShapeSphere*)bodyA->m_Shape;
    ShapeSphere* shapeB = (ShapeSphere*)bodyB->m_Shape;

    contact.ptOnA_WorldSpace = bodyA->m_Position + shapeA->m_Radius * contact.normal;
    contact.ptOnB_WorldSpace = bodyB->m_Position - shapeB->m_Radius * contact.normal;

    float radiusAB = shapeA->m_Radius + shapeB->m_Radius;
    float distanceSquared = distanceAB * distanceAB;

    if ( distanceSquared < (radiusAB * radiusAB) ) {
        return true;
    }

    return false;
}



