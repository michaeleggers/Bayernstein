#include "quick_math.h"

#include <glm/glm.hpp>
#include <stdio.h>

namespace math {

glm::vec3 TruncateVec3(const glm::vec3& vector, float max) {
    if ( glm::length(vector) > max ) {
        return glm::normalize(vector) * max;
    }
    return vector;
}

glm::vec3 ChangeOfBasis(const glm::vec3& point,
                        const glm::vec3& xAxis,
                        const glm::vec3& yAxis,
                        const glm::vec3& zAxis,
                        const glm::vec3& position) {

    glm::mat3 rotation  = glm::mat3(xAxis, yAxis, zAxis);
    glm::mat3 transform = glm::inverse(rotation);

    glm::vec3 transformedPoint = transform * point;
    transformedPoint += position;

    return transformedPoint;
}

bool CloseToZero(float value) {
    return (value < 0.0001f) && (value > -0.0001f);
}

glm::vec3 GetProjectedPoint(glm::vec3 point, glm::vec3 start, glm::vec3 end) {
    glm::vec3 ap = point - start;
    glm::vec3 ab = end - start;

    float nominator   = glm::dot(ap, ab);
    float denominator = glm::dot(ab, ab);

    if ( CloseToZero(nominator) || CloseToZero(denominator) ) {
        return end;
    }

    float result = (nominator / denominator);
    return start + result * ab;
}

bool InSegmentRange(glm::vec3 point, glm::vec3 start, glm::vec3 end) {
    glm::vec3 ap = point - start;
    glm::vec3 ab = end - start;

    float nominator   = glm::dot(ap, ab);
    float denominator = glm::dot(ab, ab);

    if ( CloseToZero(nominator) || CloseToZero(denominator) ) {
        return false;
    }

    float result = (nominator / denominator);
    return (0.0f <= result) && (result <= 1.0f);
}

} // namespace math
