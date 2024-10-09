//
// Created by me on 9/2/24.
//

#ifndef COLLISION_H
#define COLLISION_H

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "r_common.h"

struct EllipsoidCollider {
    glm::vec3	center;  // Pos in worldspace
    float		radiusA; // horizontal radius
    float		radiusB; // vertical radius
    glm::mat3   toESpace; // Maps from World to ellipsoid (unit-sphere) space
};

struct CollisionInfo {
    bool      didCollide;
    glm::vec3 hitPoint;
	float     nearestDistance;
	glm::vec3 velocity;
	glm::vec3 basePos;
};

EllipsoidCollider CreateEllipsoidColliderFromAABB(glm::vec3 mins, glm::vec3 maxs);
void CollideUnitSphereWithPlane(CollisionInfo* ci, glm::vec3 pos, Plane p, Tri tri);
glm::vec3 CollideEllipsoidWithTriPlaneRec(CollisionInfo* ci, glm::vec3 esBasePos, glm::vec3 velocity, Tri* tris, int triCount, int depth, int maxDepth);
CollisionInfo CollideEllipsoidWithTriPlane(EllipsoidCollider ec, glm::vec3 velocity, glm::vec3 gravity, TriPlane* triPlanes, int triPlaneCount);
Tri  TriToEllipsoidSpace(Tri tri, glm::mat3 toESPace);
Plane CreatePlaneFromTri(Tri tri);
bool IsPointInTriangle(glm::vec3 point, Tri tri, glm::vec3 triNormal);

#endif //COLLISION_H
