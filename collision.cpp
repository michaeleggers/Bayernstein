//
// Created by me on 9/2/24.
//

#include <stdio.h>
#include <vector>

#include <glm/gtx/norm.hpp>

#include "collision.h"
#include "globals.h"
#include "r_common.h"

EllipsoidCollider CreateEllipsoidColliderFromAABB(glm::vec3 mins, glm::vec3 maxs)
{
    float             width  = glm::abs(maxs.x - mins.x);
    float             height = glm::abs(maxs.z - mins.z);
    EllipsoidCollider result{};
    float             radiusA = width / 2.0f;
    float             radiusB = height / 2.0f;
    result.radiusA            = radiusA;
    result.radiusB            = radiusB;
    result.center             = glm::vec3(0.0f);
    glm::vec3 s               = glm::vec3(1.0f / radiusA, 1.0f / radiusA, 1.0f / radiusB);
    glm::mat3 invESpace       = glm::scale(glm::mat4(1.0f), s);
    result.toESpace           = invESpace;

    return result;
}

// Assumes Triangle in CCW order.
glm::vec3 ConstructNormalToTriLineSegment(glm::vec3 a, glm::vec3 b, glm::vec3 planeNormal)
{
    glm::vec3 AB  = glm::normalize(b - a);
    glm::vec3 nAB = glm::normalize(glm::cross(planeNormal, AB));

    return nAB;
}

float SignedDistancePointToPlane(glm::vec3 pt, glm::vec3 ptOnPlane, glm::vec3 planeNormal)
{
    glm::vec3 pVec = pt - ptOnPlane;
    float     sD   = glm::dot(pVec, planeNormal);

    return sD;
}

bool IsPointInTriangle(glm::vec3 point, Tri tri, glm::vec3 triNormal)
{
    glm::vec3 nAB       = ConstructNormalToTriLineSegment(tri.a.pos, tri.b.pos, triNormal);
    float     sdABplane = SignedDistancePointToPlane(point, tri.a.pos, nAB);
    // printf("sdABplane: %f\n", sdABplane);
    if ( sdABplane <= 0.0f )
    {
        return false;
    }

    glm::vec3 nBC       = ConstructNormalToTriLineSegment(tri.b.pos, tri.c.pos, triNormal);
    float     sdBCplane = SignedDistancePointToPlane(point, tri.b.pos, nBC);
    if ( sdBCplane <= 0.0f )
    {
        return false;
    }

    glm::vec3 nCA       = ConstructNormalToTriLineSegment(tri.c.pos, tri.a.pos, triNormal);
    float     sdCAplane = SignedDistancePointToPlane(point, tri.c.pos, nCA);
    if ( sdCAplane <= 0.0f )
    {
        return false;
    }

    return true;
}

// If a > b then the two will be swapped.
// Otherwise, nothing happens.
void SortFloats(float* a, float* b)
{
    if ( *a > *b )
    {
        float tmp = *a;
        *a        = *b;
        *b        = tmp;
    }
}

void SortDoubles(double* a, double* b)
{
    if ( *a > *b )
    {
        double tmp = *a;
        *a         = *b;
        *b         = tmp;
    }
}

#define SortValues(a, b, t)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        t(&a, &b);                                                                                                     \
    } while ( 0 );

bool GetSmallestRoot(float a, float b, float c, float maxRoot, float* root)
{
    float D = b * b - 4.0f * a * c;
    if ( D < 0.0f )
    {
        return false;
    }

    float sqrtD = glm::sqrt(D);
    float denom = 1.0f / (2.0f * a);
    float r0    = (-b - sqrtD) * denom;
    float r1    = (-b + sqrtD) * denom;

    // Make sure we have the lowest solution in r0.
    SortValues(r0, r1, SortFloats);

    if ( r0 > 0.0f && r0 < maxRoot )
    {
        *root = r0;
        return true;
    }

    // r0 could be a negative solution. In plain math this is ok,
    // but we are only interested in values that are between
    // 0 and maxRoot (which is 1 at maximum).
    if ( r1 > 0.0f && r1 < maxRoot )
    {
        *root = r1;
        return true;
    }

    return false;
}

bool IsPointOnLineSegment(glm::vec3 p, glm::vec3 a, glm::vec3 b)
{
    // Check if the intersection point between sphere and plane we found
    // earlier lies on one of the 3 triangle's edges.
    // Firstly, check if the intersection point is on the line.
    glm::vec3 edge        = glm::normalize(b - a);
    glm::vec3 pVec        = glm::normalize(p - a);
    float     edgeDotPVec = glm::dot(edge, pVec);
    printf("edgeDotPVec = %f\n", edgeDotPVec);
    if ( edgeDotPVec >= HKD_EPSILON )
    { // intersection point lies on line
        float lenE0 = glm::length(edge);
        float lenP  = glm::length(pVec);
        if ( lenP < lenE0 )
        {
            return true;
        }
    }

    return false;
}

bool TraceRayAgainstUnitSphere(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 sphereCenter)
{
    glm::vec3 rayPosToSphereCenter = sphereCenter - rayPos;
    glm::vec3 normRayDir           = glm::normalize(rayDir);
    float     projection           = glm::dot(normRayDir, rayPosToSphereCenter);

    // Sphere is 'behind' raypos
    if ( projection < 0.0f )
    {
        return false;
    }

    glm::vec3 rayPosToProjectedPoint  = rayPos + projection * normRayDir;
    glm::vec3 projPointToSphereCenter = sphereCenter - rayPosToProjectedPoint;
    float     length                  = glm::length(projPointToSphereCenter);

    if ( length < 1.0f )
    {
        return true;
    }

    return false;
}

bool TraceRayAgainstEllipsoid(glm::vec3 rayPos, glm::vec3 rayDir, EllipsoidCollider ec)
{
    glm::vec3 scaleToESpace(ec.toESpace[ 0 ][ 0 ], ec.toESpace[ 1 ][ 1 ], ec.toESpace[ 2 ][ 2 ]);

    glm::vec3 rayPosESpace          = scaleToESpace * rayPos;
    glm::vec3 rayDirESpace          = scaleToESpace * rayDir;
    glm::vec3 ellipsoidCenterESpace = scaleToESpace * ec.center;

    return TraceRayAgainstUnitSphere(rayPosESpace, rayDirESpace, ellipsoidCenterESpace);
}

// NOTE: Logic is a bit different from Fauerby's paper as
// I was not able to derive the formula like it is in the paper.
// So this one is being used: https://pythno.org/blog/posts/sphere_vs_linesegment/
// But it is essentially the same, just with flipped signs (check the blogpost for more info).
bool CheckSweptSphereVsLinesegment(glm::vec3  p0,
                                   glm::vec3  p1,
                                   glm::vec3  sphereBase,
                                   glm::vec3  velocity,
                                   float      maxT,
                                   float*     out_newT,
                                   glm::vec3* out_hitPoint)
{
    // Check sphere against tri's line-segments

    glm::vec3 e                = p1 - p0;
    float     eSquaredLength   = glm::length2(e);
    float     vSquaredLength   = glm::length2(velocity);
    glm::vec3 baseToVertex     = sphereBase - p0;
    float     eDotVel          = glm::dot(e, velocity);
    float     eDotBaseToVertex = glm::dot(e, baseToVertex);

    float a = eSquaredLength * vSquaredLength - eDotVel * eDotVel;
    float b = eSquaredLength * 2.0f * glm::dot(velocity, baseToVertex) - 2.0f * (eDotVel * eDotBaseToVertex);
    float c = eSquaredLength * (glm::length2(baseToVertex) - 1.0f) - eDotBaseToVertex * eDotBaseToVertex;

    float t;
    if ( GetSmallestRoot(a, b, c, maxT, &t) )
    {
        // Now check if hitpoint is withing the line segment.
        float f = (eDotVel * t + eDotBaseToVertex) / eSquaredLength;
        if ( f >= 0.0f && f <= 1.0f )
        {
            *out_newT     = t;
            *out_hitPoint = p0 + f * e;

            return true;
        }
    }

    return false;
}

static void CollideUnitSphereWithTri(CollisionInfo* ci, const Tri& tri)
{
    Plane p{};
    CreatePlaneFromTri(&p, tri);
    //glm::vec3 normal(p.normal);
    glm::vec3 ptOnPlane = p.d * p.normal;
    glm::vec3 basePos   = ci->basePos;
    glm::vec3 velocity  = ci->velocity;

    if ( glm::dot(velocity, p.normal) >= 0.0f ) return;
    // Signed distance from plane to unit sphere's center
    float sD = glm::dot(p.normal, basePos - ptOnPlane);

    // Project velocity along the plane normal
    float velDotNormal = glm::dot(p.normal, velocity);

    bool  foundCollision  = false;
    bool  embeddedInPlane = false;
    float t0, t1;
    if ( glm::abs(velDotNormal) == 0.0f )
    { // Sphere travelling parallel to the plane or it is in the plane

        // Distance from unit sphere center to plane is greater than 1 => no intersection!
        if ( glm::abs(sD) >= 1.0f )
        {
            return;
        }

        t0              = 0.0f;
        t1              = 1.0f;
        embeddedInPlane = true;
    }
    else
    { // N Dot D not 0! There could be an intersection!

        // Calculate t0, that is the time when the unit sphere hits the
        // front face of the plane.
        t0 = (1.0f - sD) / velDotNormal;

        // As above, calculate t1, that is the time when the unit sphere
        // hits the back face of the plane.
        t1 = (-1.0f - sD) / velDotNormal;

        // printf("t0: %f, t1: %f\n", t0, t1);

        // t0, t1 marks the intersection interval. Make sure
        // t0 < t1 because depending on what side of the Plane
        // the sphere is, t0 might not be the smallest value.
        // But we need the smallest because it is the maximum we can
        // travel the sphere along the velocity vector before a collision happens.
        if ( t0 > t1 )
        {
            float tmp = t0;
            t0        = t1;
            t1        = tmp;
        }

        if ( t0 > 1.0f || t1 < 0.0f )
        { // No collision
            return;
        }

        t0 = glm::clamp(t0, 0.0f, 1.0f);
        t1 = glm::clamp(t1, 0.0f, 1.0f);
    }

    // Collision could be at the front side of the plane.
    // This is only possible when the intersection point is not embedded inside
    // the plane.
    glm::vec3 hitPoint;
    float     t = 1.0f;
    if ( !embeddedInPlane )
    {
        // Check if the intersection is INSIDE the triangle.
        glm::vec3 intersectionPoint = basePos + t0 * velocity - p.normal;
        if ( IsPointInTriangle(intersectionPoint, tri, p.normal) )
        { // TODO: Rename function!
            foundCollision = true;
            hitPoint       = intersectionPoint;
            //printf("Point inside tri side planes.\n");
            t = t0;
            //printf("IsPointInTriangle: true\n");
        }
        else
        {
            //printf("Point outside tri side planes.\n");
        }
    }

    // Check if collision with one of the 3 vertices of the  triangle.
    // Can only happen if we did not collide previously with the 'inside'
    // of the triangle's side planes.
    // The Equation we have to solve is:
    // ( C(t) - p )^2 = 1, that is when the sphere collided with the tri's vertex.
    // where C(t) is the current pos of the sphere on its velocity vector:
    // C(t) = basePos + t * velocity
    // p is one of the vertices of the tri.
    if ( !foundCollision )
    {
        float a = glm::length2(velocity);
        float newT;
        // Check point A
        float b = 2.0f * (glm::dot(velocity, (basePos - tri.a.pos)));
        float c = glm::distance2(tri.a.pos, basePos) - 1.0f;
        // Find smallest solution, if available
        if ( GetSmallestRoot(a, b, c, t, &newT) )
        {
            t              = newT;
            foundCollision = true;
            hitPoint       = tri.a.pos;
        }

        // Check point B
        b = 2.0f * (glm::dot(velocity, (basePos - tri.b.pos)));
        c = glm::distance2(tri.b.pos, basePos) - 1.0f;
        // Find smallest solution, if available
        if ( GetSmallestRoot(a, b, c, t, &newT) )
        {
            t              = newT;
            foundCollision = true;
            hitPoint       = tri.b.pos;
        }

        // Check point C
        b = 2.0f * (glm::dot(velocity, (basePos - tri.c.pos)));
        c = glm::distance2(tri.c.pos, basePos) - 1.0f;
        // Find smallest solution, if available
        if ( GetSmallestRoot(a, b, c, t, &newT) )
        {
            t              = newT;
            foundCollision = true;
            hitPoint       = tri.c.pos;
        }

        // Check sphere against tri's line-segments

        if ( CheckSweptSphereVsLinesegment(tri.a.pos, tri.b.pos, basePos, velocity, t, &newT, &hitPoint) )
        {
            foundCollision = true;
            t              = newT;
        }

        if ( CheckSweptSphereVsLinesegment(tri.b.pos, tri.c.pos, basePos, velocity, t, &newT, &hitPoint) )
        {
            foundCollision = true;
            t              = newT;
        }

        if ( CheckSweptSphereVsLinesegment(tri.c.pos, tri.a.pos, basePos, velocity, t, &newT, &hitPoint) )
        {
            foundCollision = true;
            t              = newT;
        }
    }

    if ( foundCollision )
    {
        float distanceToHitpoint
            = t
              * glm::length(
                  velocity); // furthest the sphere can travel along its velocity vector until collision happens
        if ( !ci->didCollide || (ci->nearestDistance > distanceToHitpoint) )
        {
            ci->didCollide      = true;
            ci->nearestDistance = distanceToHitpoint;
            ci->hitPoint        = hitPoint;

            // NOTE: This could help to counter 'false positives'
            // but as not proven to be numerically stable yet.
            // This code is just there as a reminder that
            // *maybe* something could be done here.
            /*
            if (embeddedInPlane) {
                ci->nearestDistance = -DOD_VERY_CLOSE_DIST;
                ci->hitPoint -= DOD_VERY_CLOSE_DIST*p.normal;
            }
            */
        }
    }
}

static std::vector<Tri> g_MapTrisCache;
void                    InitMapTrisCache(size_t numTris)
{
    g_MapTrisCache.resize(numTris);
}

CollisionInfo CollideEllipsoidWithMapTris(EllipsoidCollider                 ec,
                                          glm::vec3                         velocity,
                                          glm::vec3                         gravity,
                                          MapTri*                           tris,
                                          int                               triCount,
                                          std::vector<std::vector<MapTri>*> brushMapTris)
{
    // Convert to ellipsoid space
    glm::vec3 scaleToESpace(ec.toESpace[ 0 ][ 0 ], ec.toESpace[ 1 ][ 1 ], ec.toESpace[ 2 ][ 2 ]);

    for ( int i = 0; i < triCount; i++ )
    {
        Tri tri = tris[ i ].tri;
        TriToEllipsoidSpace(&tri, scaleToESpace);
        g_MapTrisCache[ i ] = tri;
    }
    int currentTriCount = triCount;

    // Also do this for the brush tris
    for ( int brushIndex = 0; brushIndex < brushMapTris.size(); brushIndex++ )
    {
        std::vector<MapTri>* pMapTris = brushMapTris[ brushIndex ];
        for ( int j = 0; j < pMapTris->size(); j++ )
        {
            Tri tri = pMapTris->at(j).tri;
            TriToEllipsoidSpace(&tri, scaleToESpace);
            g_MapTrisCache[ currentTriCount + j ] = tri;
        }
        currentTriCount += pMapTris->size();
    }

    glm::vec3 esVelocity = scaleToESpace * velocity;
    glm::vec3 esBasePos  = scaleToESpace * ec.center;

    // From now on the Radius of the ellipsoid is 1.0 in X, Y, Z.
    // Thus, it is a unit sphere.

    CollisionInfo ci{};
    ci.didCollide      = false;
    ci.nearestDistance = 0.0f;
    ci.velocity        = esVelocity;
    ci.hitPoint        = glm::vec3(0.0f);
    ci.basePos         = esBasePos;

    glm::vec3 newPos
        = CollideEllipsoidWithTrisRec(&ci, esBasePos, esVelocity, g_MapTrisCache.data(), g_MapTrisCache.size(), 0, 5);

    glm::vec3 esGravity = scaleToESpace * gravity;
    ci.velocity         = esGravity;
    ci.basePos          = newPos;
    ci.nearestDistance  = 0.0f;
    ci.didCollide       = false;
    newPos = CollideEllipsoidWithTrisRec(&ci, newPos, esGravity, g_MapTrisCache.data(), g_MapTrisCache.size(), 0, 5);

    glm::vec3 invESpace(1.0f / scaleToESpace);
    ci.velocity = invESpace * ci.velocity;
    ci.hitPoint = invESpace * ci.hitPoint;
    ci.basePos  = invESpace * newPos;

    return ci;
}

// Assume all data in ci to be in ellipsoid space, that is, a unit-sphere. Same goes for esBasePos.
glm::vec3 CollideEllipsoidWithTrisRec(
    CollisionInfo* ci, glm::vec3 esBasePos, glm::vec3 velocity, Tri* tris, int triCount, int depth, int maxDepth)
{
    if ( depth > maxDepth ) return esBasePos;

    ci->didCollide = false;
    ci->velocity   = velocity;
    ci->basePos    = esBasePos;

    for ( int i = 0; i < triCount; i++ )
    {
        const Tri& tri = tris[ i ];
        CollideUnitSphereWithTri(ci, tri);
    }

    if ( !ci->didCollide )
    {
        return esBasePos + velocity;
    }

    glm::vec3 destinationPos = esBasePos + velocity;
    glm::vec3 newBasePos     = esBasePos;

    if ( ci->nearestDistance >= DOD_VERY_CLOSE_DIST )
    {
        glm::vec3 v      = velocity;
        glm::vec3 vNorm  = glm::normalize(v);
        float     length = glm::abs(ci->nearestDistance - DOD_VERY_CLOSE_DIST); // abs should not be neccessary
        newBasePos       = ci->basePos + length * vNorm;
        ci->hitPoint -= DOD_VERY_CLOSE_DIST * vNorm;
    }

    Plane slidingPlane{};
    slidingPlane.p              = ci->hitPoint;
    slidingPlane.normal         = glm::normalize(newBasePos - ci->hitPoint);
    float     distance          = glm::dot(destinationPos - slidingPlane.p, slidingPlane.normal);
    glm::vec3 newDestinationPos = destinationPos - distance * slidingPlane.normal;

    glm::vec3 newVelocity = newDestinationPos - ci->hitPoint;

    if ( glm::length(newVelocity) < DOD_VERY_CLOSE_DIST )
    {
        return newBasePos;
    }

    return CollideEllipsoidWithTrisRec(ci, newBasePos, newVelocity, tris, triCount, depth + 1, maxDepth);
}

void TriToEllipsoidSpace(Tri* tri, glm::vec3 scaleToESpace)
{
    tri->a.pos = scaleToESpace * tri->a.pos;
    tri->b.pos = scaleToESpace * tri->b.pos;
    tri->c.pos = scaleToESpace * tri->c.pos;
}

// NOTE: This is to prevent allocating new memory every
// time 'PushTouch' is being called. This is a temporary
// fix. It increases the framerate on a release build
// from around 400FPS to 2000FPS on a recent GPU.
// However this also highly depends on the STL implementation.
static std::vector<Tri> g_esTriMemory;
CollisionInfo           PushTouch(EllipsoidCollider ec, glm::vec3 velocity, MapTri* tris, int triCount)
{
    g_esTriMemory.clear();

    glm::vec3 scaleToESpace(ec.toESpace[ 0 ][ 0 ], ec.toESpace[ 1 ][ 1 ], ec.toESpace[ 2 ][ 2 ]);
    for ( int i = 0; i < triCount; i++ )
    {
        Tri tri = tris[ i ].tri;
        TriToEllipsoidSpace(&tri, scaleToESpace);
        g_esTriMemory.push_back(tri);
    }
    glm::vec3 esBasePos  = scaleToESpace * ec.center;
    glm::vec3 esVelocity = scaleToESpace * velocity;

    CollisionInfo ci;
    ci.didCollide      = false;
    ci.nearestDistance = 0.0f;
    ci.velocity        = esVelocity;
    ci.hitPoint        = glm::vec3(0.0f);
    ci.basePos         = esBasePos;

    for ( int i = 0; i < triCount; i++ )
    {
        CollideUnitSphereWithTri(&ci, g_esTriMemory[ i ]);
        if ( ci.didCollide )
        {
            break;
        }
    }

    glm::vec3 invESpace(1.0f / scaleToESpace);
    ci.hitPoint = invESpace * ci.hitPoint;
    ci.velocity = invESpace * ci.velocity;

    return ci;
}
