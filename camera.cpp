#include "camera.h"

#include "globals.h"

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"
#include "dependencies/glm/gtx/vector_angle.hpp"

Camera::Camera(glm::vec3 pos)
{
    m_Pos = pos;
    m_Forward = DOD_WORLD_FORWARD;
    m_Side = DOD_WORLD_RIGHT;
    m_Up = DOD_WORLD_UP;
    m_Orientation = glm::angleAxis(glm::radians(0.0f), m_Up);
}

/*
void Camera::SetForward(glm::vec3 forward) {
    m_Forward = forward;
    m_Side = glm::cross( m_Forward, DOD_WORLD_UP );
    m_Up = glm::cross( 
}
*/

void Camera::Pan(glm::vec3 direction)
{
    m_Pos += direction;
}

void Camera::RotateAroundWorldUp(float angle)
{
    glm::quat orientation = glm::angleAxis( glm::radians(angle), DOD_WORLD_UP );
    m_Up = glm::rotate(orientation, m_Up);
    m_Forward = glm::rotate(orientation, m_Forward);
    m_Side = glm::rotate(orientation, m_Side);
    m_Orientation *= orientation;
}

void Camera::RotateAroundSide(float angle)
{
    glm::quat orientation = glm::angleAxis(glm::radians(angle), glm::normalize(m_Side));
    m_Up = glm::rotate(orientation, m_Up);
    m_Forward = glm::rotate(orientation, m_Forward);
    m_Orientation *= orientation;
}

void Camera::SetOrientationFromAngle(float angle, glm::vec3 axis) {
    glm::quat orientation = glm::angleAxis( glm::radians(angle), axis );
    m_Up = glm::rotate( orientation, DOD_WORLD_UP );
    m_Forward = glm::rotate( orientation, DOD_WORLD_FORWARD );
    m_Side = glm::rotate( orientation, DOD_WORLD_RIGHT );
    m_Orientation = orientation;
}

void Camera::LookAt(glm::vec3 target) {
    // Create new camera frame (Gram-Schmidt orthogonalization).
    glm::vec3 newForward = glm::normalize(target - m_Pos);
    glm::vec3 newSide = glm::cross( newForward, DOD_WORLD_UP );
    glm::vec3 newUp = glm::cross( newSide, newForward );
    glm::mat3 newFrame = glm::mat3(newForward,
                                   newSide,
                                   newUp);
    m_Forward = newForward;
    m_Side = newSide;
    m_Up = newUp;
    glm::quat m_Orientation = glm::quat_cast(newFrame);
}

glm::mat4 Camera::ViewMatrix(void)
{
    glm::vec3 center = m_Pos + m_Forward;
    return glm::lookAt(m_Pos, center, m_Up);
}


