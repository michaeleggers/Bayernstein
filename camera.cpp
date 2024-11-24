#include "camera.h"

#include "globals.h"

Camera::Camera(glm::vec3 pos)
{
	m_Pos = pos;
	m_Forward = DOD_WORLD_FORWARD;
	m_Side = DOD_WORLD_RIGHT;
	m_Up = DOD_WORLD_UP;
	m_Orientation = glm::angleAxis(glm::radians(0.0f), m_Up);
}

void Camera::Pan(glm::vec3 direction)
{
	m_Pos += direction;
}

void Camera::Rotate(glm::quat quat)
{
	m_Orientation *= quat;
	m_Up = glm::rotate(m_Orientation, m_Up);
	m_Forward = glm::rotate(m_Orientation, m_Forward);
	m_Side = glm::rotate(m_Orientation, m_Side);
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

glm::mat4 Camera::ViewMatrix(void)
{
	glm::vec3 center = m_Pos + m_Forward;
	return glm::lookAt(m_Pos, center, m_Up);
}

