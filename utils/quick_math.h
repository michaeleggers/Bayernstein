#ifndef _UTILS_MATH_H_
#define _UTILS_MATH_H_
#include <glm/glm.hpp>

namespace math {
glm::vec3 Truncate(const glm::vec3& vector, float max);

glm::vec3 ChangeOfBasis(const glm::vec3& point, const glm::vec3& xAxis, const glm::vec3& yAxis, const glm::vec3& zAxis);

} // namespace math
#endif