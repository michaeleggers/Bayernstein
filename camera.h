#ifndef _CAMERA_H_
#define _CAMERA_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

class Camera {
  public:
    Camera(glm::vec3 pos = glm::vec3(0.0f));

    void Pan(glm::vec3 direction);
    void RotateAroundWorldUp(float angle);
    void RotateAroundSide(float angle);
    void SetOrientationFromAngle(float angle, glm::vec3 axis);
    void LookAt(glm::vec3 target);

    glm::mat4 ViewMatrix(void);

    glm::vec3 m_Pos;
    glm::vec3 m_Forward;
    glm::vec3 m_Side;
    glm::vec3 m_Up;
    glm::quat m_Orientation;
};

#endif
