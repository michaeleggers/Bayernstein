#include <glm/glm.hpp>

#include "../r_common.h"
#include "quick_math.h"

namespace math
{

glm::vec3 TruncateVec3(const glm::vec3& vector, float max)
{
    if ( glm::length(vector) > max )
    {
        return glm::normalize(vector) * max;
    }
    return vector;
}

glm::vec3 ChangeOfBasis(const glm::vec3& point,
                        const glm::vec3& xAxis,
                        const glm::vec3& yAxis,
                        const glm::vec3& zAxis,
                        const glm::vec3& position)
{

    glm::mat3 rotation  = glm::mat3(xAxis, yAxis, zAxis);
    glm::mat3 transform = glm::inverse(rotation);

    glm::vec3 transformedPoint = transform * point;
    transformedPoint += position;

    return transformedPoint;
}

bool CloseToZero(float value)
{
    return (value < 0.0001f) && (value > -0.0001f);
}

glm::vec3 GetProjectedPoint(glm::vec3 point, glm::vec3 start, glm::vec3 end)
{
    glm::vec3 ap = point - start;
    glm::vec3 ab = end - start;

    float nominator   = glm::dot(ap, ab);
    float denominator = glm::dot(ab, ab);

    if ( CloseToZero(nominator) || CloseToZero(denominator) )
    {
        return end;
    }

    float result = (nominator / denominator);
    return start + result * ab;
}

bool InSegmentRange(glm::vec3 point, glm::vec3 start, glm::vec3 end)
{
    glm::vec3 ap = point - start;
    glm::vec3 ab = end - start;

    float nominator   = glm::dot(ap, ab);
    float denominator = glm::dot(ab, ab);

    if ( CloseToZero(nominator) || CloseToZero(denominator) )
    {
        return false;
    }

    float result = (nominator / denominator);
    return (0.0f <= result) && (result <= 1.0f);
}

// Mcam: transform of camera in object space to worldspace.
Frustum BuildFrustum(const glm::mat4& Mcam, float g, float s, float n, float f)
{
    Frustum frustum{};
    frustum.projDistance = g;
    frustum.aspectRatio  = s;
    frustum.near         = n;
    frustum.far          = f;

    // Create vertices for near plane
    float z               = n / g;
    float x               = z * s;
    frustum.vertices[ 0 ] = { Mcam * glm::vec4(x, n, z, 1.0f) };
    frustum.vertices[ 1 ] = { Mcam * glm::vec4(x, n, -z, 1.0f) };
    frustum.vertices[ 2 ] = { Mcam * glm::vec4(-x, n, -z, 1.0f) };
    frustum.vertices[ 3 ] = { Mcam * glm::vec4(-x, n, z, 1.0f) };

    // Create vertices for far plane
    z                     = f / g;
    x                     = z * s;
    frustum.vertices[ 4 ] = { Mcam * glm::vec4(x, f, z, 1.0f) };
    frustum.vertices[ 5 ] = { Mcam * glm::vec4(x, f, -z, 1.0f) };
    frustum.vertices[ 6 ] = { Mcam * glm::vec4(-x, f, -z, 1.0f) };
    frustum.vertices[ 7 ] = { Mcam * glm::vec4(-x, f, z, 1.0f) };

    // Create the frustum's planes
    glm::mat4 toWorldSpace = glm::inverse(Mcam);

    float mx = 1.0f / glm::sqrt(g * g + s * s);
    float my = 1.0f / glm::sqrt(g * g + 1.0f);

    // Original code from Lengyel (he uses y down, z into screen, x right)
    //frustum.planes[ 0 ] = toWorldSpace * Plane(glm::vec3(-g * mx, 0.0f, s * mx), 0.0f); // right
    //frustum.planes[ 1 ] = toWorldSpace * Plane(glm::vec3(0.0f, g * my, my), 0.0f);
    //frustum.planes[ 2 ] = toWorldSpace * Plane(glm::vec3(g * mx, 0.0f, s * mx), 0.0f);
    //frustum.planes[ 3 ] = toWorldSpace * Plane(glm::vec3(0.0f, -g * my, my), 0.0f);

    // Lengyel's code change to our coord system:
    frustum.planes[ 0 ] = toWorldSpace * Plane(glm::vec3(-g * mx, s * mx, 0.0f), 0.0f); // right
    frustum.planes[ 1 ] = toWorldSpace * Plane(glm::vec3(0.0f, my, -g * my), 0.0f);     //top
    frustum.planes[ 2 ] = toWorldSpace * Plane(glm::vec3(g * mx, s * mx, 0.0f), 0.0f);  // left
    frustum.planes[ 3 ] = toWorldSpace * Plane(glm::vec3(0.0f, my, g * my), 0.0f);      // bottom

    // Near and Far planes
    float d             = glm::dot(Mcam[ 2 ], Mcam[ 3 ]);
    frustum.planes[ 4 ] = Plane(Mcam[ 2 ], -(d + n));
    frustum.planes[ 5 ] = Plane(-Mcam[ 2 ], d + f);

    return frustum;
}

} // namespace math
