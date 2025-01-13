#ifndef _UTILS_MATH_H_
#define _UTILS_MATH_H_
#include <glm/glm.hpp>

namespace math
{

struct Frustum
{
    float projDistance;
    float aspectRatio;
    float near;
    float far;

    Plane  planes[ 6 ];
    Vertex vertices[ 8 ];
};

glm::vec3 TruncateVec3(const glm::vec3& vector, float max);
glm::vec3 ChangeOfBasis(const glm::vec3& point,
                        const glm::vec3& xAxis,
                        const glm::vec3& yAxis,
                        const glm::vec3& zAxis,
                        const glm::vec3& position);
glm::vec3 GetProjectedPoint(glm::vec3 point, glm::vec3 start, glm::vec3 end);
bool      InSegmentRange(glm::vec3 point, glm::vec3 start, glm::vec3 end);
Frustum   BuildFrustum(const glm::mat4& Mcam, float g, float s, float n, float f);

} // namespace math

#endif
