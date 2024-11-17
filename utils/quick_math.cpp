#include "quick_math.h"
#include <glm/glm.hpp>

namespace math {

glm::vec3 TruncateVec3(const glm::vec3& vector, float max) {
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

    glm::mat3 rotation  = glm::mat3(normXAxis, normYAxis, normZAxis);
    glm::mat3 transform = glm::inverse(rotation);

    glm::vec3 transformedPoint = transform * point;

    return transformedPoint;
}

glm::vec3 GetNormalPoint(glm::vec3 p, glm::vec3 a, glm::vec3 b) {
    glm::vec3 ap = p - a;
    glm::vec3 ab = b - a;

    float nominator   = glm::dot(ap, ab);
    float denominator = glm::dot(ab, ab);

    float result = (nominator / denominator);
    return result * glm::normalize(ab);
}

bool InSegmentRange(glm::vec3 start, glm::vec3 end, glm::vec3 point) {
    float segmentLength = glm::distance(start, end);
    float startToPoint  = glm::distance(start, point);
    float endToPoint    = glm::distance(end, point);
    return startToPoint + endToPoint <= segmentLength;
}

} // namespace math
