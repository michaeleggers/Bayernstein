#include "quick_math.h"
#include <glm/glm.hpp>

namespace math {

glm::vec3 Truncate(const glm::vec3& vector, float max) {
    if ( glm::length(vector) > max ) {
        return glm::normalize(vector) * max;
    }
    return vector;
}

glm::vec3
ChangeOfBasis(const glm::vec3& point, const glm::vec3& xAxis, const glm::vec3& yAxis, const glm::vec3& zAxis) {
    // Normalize the vectors to ensure no scaling is included
    glm::vec3 normXAxis = glm::normalize(xAxis);
    glm::vec3 normYAxis = glm::normalize(yAxis);
    glm::vec3 normZAxis = glm::normalize(zAxis);

    glm::mat3 rotation = glm::mat3(normXAxis, normYAxis, normZAxis);
    glm::mat3 transform = glm::inverse(rotation);

    glm::vec3 transformedPoint = transform * point;

    return transformedPoint;
}
} // namespace math
