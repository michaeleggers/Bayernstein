#include "math.h"
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

    glm::mat4 rotation = glm::mat4(glm::vec4(normXAxis, 0.0f),
                                   glm::vec4(normZAxis, 0.0f), // NOTE: ZAxis is the new YAxis
                                   glm::vec4(normYAxis, 0.0f),
                                   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 transform = glm::inverse(rotation);

    glm::vec4 transformedPoint = transform * glm::vec4(point, 1.0f);

    return glm::vec3(transformedPoint);
}
} // namespace math
