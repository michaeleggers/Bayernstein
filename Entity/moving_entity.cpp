#include "moving_entity.h"

glm::vec3 MovingEntity::GetVelocity() const {
    return m_Velocity;
}

glm::vec3 MovingEntity::GetForward() const {
    return m_Forward;
}

glm::vec3 MovingEntity::GetSide() const {
    return m_Side;
}

glm::vec3 MovingEntity::GetUp() const {
    return m_Up;
}
float MovingEntity::GetMaxForce() const {
    return m_MaxForce;
}

float MovingEntity::GetMaxSpeed() const {
    return m_MaxSpeed;
}
