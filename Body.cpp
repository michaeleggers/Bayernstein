//
// Created by me on 8/12/24.
//

#include "Body.h"

glm::vec3 Body::GetCenterOfMassWorldSpace() const {
    glm::vec3 centerOfMass = m_Shape->GetCenterOfMass();
    glm::mat3 r = glm::mat3_cast(m_Orientation);
    centerOfMass = r * centerOfMass;
    return m_Position + centerOfMass;
}

glm::vec3 Body::GetCenterOfMassModelSpace() const {
    return m_Shape->GetCenterOfMass();
}

glm::vec3 Body::WorldSpaceToBodySpace(glm::vec3 &pt) {
    glm::vec3 tmp = pt - GetCenterOfMassWorldSpace();

    // m_Orientation is the rotation in WorldSpace.
    // So we need to rotate back to get the original pt in BodySpace
    glm::quat inverseOrient = inverse(m_Orientation);

    glm::mat3 r = glm::mat3_cast(inverseOrient);
    glm::vec3 bodySpace = r * tmp;
    return bodySpace;
}

glm::vec3 Body::BodySpaceToWorldSpace(glm::vec3 &pt) {
    glm::mat3 r = glm::mat3_cast(m_Orientation);
    glm::vec3 ptWorld = r * pt;
    return GetCenterOfMassWorldSpace() + ptWorld;
}

void Body::ApplyImpulseLinear(glm::vec3 &impulse) {
    if (m_InvMass <= 0.0f) return;

    m_LinearVelocity += impulse * m_InvMass;
}
