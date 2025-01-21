//
// Created by me on 9/2/24.
//

#ifndef _COLLISION_H_
#define _COLLISION_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"

#include "r_common.h"

struct EllipsoidCollider
{
    glm::vec3 center;   // Pos in worldspace
    float     radiusA;  // horizontal radius
    float     radiusB;  // vertical radius
    glm::mat3 toESpace; // Maps from World to ellipsoid (unit-sphere) space
};

struct CollisionInfo
{
    bool      didCollide;
    glm::vec3 hitPoint;
    float     nearestDistance;
    glm::vec3 velocity;
    glm::vec3 basePos;
};

bool              TraceRayAgainstUnitSphere(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 sphereCenter);
bool              TraceRayAgainstEllipsoid(glm::vec3 rayPos, glm::vec3 rayDir, EllipsoidCollider ec);
EllipsoidCollider CreateEllipsoidColliderFromAABB(glm::vec3 mins, glm::vec3 maxs);
//void              CollideUnitSphereWithPlane(CollisionInfo* ci, glm::vec3 pos, Plane p, Tri tri);
glm::vec3         CollideEllipsoidWithTrisRec(
            CollisionInfo* ci, glm::vec3 esBasePos, glm::vec3 velocity, Tri* tris, int triCount, int depth, int maxDepth);
CollisionInfo CollideEllipsoidWithMapTris(EllipsoidCollider                 ec,
                                          glm::vec3                         velocity,
                                          glm::vec3                         gravity,
                                          MapTri*                           tris,
                                          int                               triCount,
                                          std::vector<std::vector<MapTri>*> brushMapTris);
void          TriToEllipsoidSpace(Tri* tri, glm::vec3 scaleToESpace);
bool          IsPointInTriangle(glm::vec3 point, Tri tri, glm::vec3 triNormal);
// PushTouch will *only* trigger a collision if the ellipsoid is moving by a non-zero velocity vector.
CollisionInfo PushTouch(EllipsoidCollider ec, glm::vec3 velocity, MapTri* tris, int triCount);

#endif // _COLLISION_H_
