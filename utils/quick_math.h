#ifndef _UTILS_MATH_H_
#define _UTILS_MATH_H_
#include <glm/glm.hpp>

namespace math {

glm::vec3 TruncateVec3(const glm::vec3& vector, float max);
glm::vec3 ChangeOfBasis(const glm::vec3& point, const glm::vec3& xAxis, const glm::vec3& yAxis, const glm::vec3& zAxis);
glm::vec3 GetNormalPoint(glm::vec3 p, glm::vec3 a, glm::vec3 b);
bool InSegmentRange(glm::vec3 start, glm::vec3 end, glm::vec3 point);

} // namespace math
#endif
