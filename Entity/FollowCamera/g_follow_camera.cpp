#include "g_follow_camera.h"

#include <assert.h>

#include "../../camera.h"

CFollowCamera::CFollowCamera(BaseGameEntity* target)
    : BaseGameEntity(ET_CAMERA) {

    assert(target != nullptr);

    m_Target   = target;
    m_Position = target->m_Position;
    m_Camera   = Camera(m_Position);
    // FIX: The player model (target) was modeled to face to -y which
    // is not the default! When we use actual models we should pay
    // attention to model our characters to face +y to not have to
    // do fixes in code.
    m_RotationAngle = 180.0f;
}

CFollowCamera::~CFollowCamera() {}

void CFollowCamera::Update() {
    glm::vec3 targetPos     = m_Target->m_Position;
    float     rotationAngle = m_Target->m_RotationAngle;

    //printf("rotationAngle: %f\n", rotationAngle);

    m_Camera.SetOrientationFromAngle(m_RotationAngle + rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    //m_Camera.RotateAroundUp(rotationAngle);

    // Set this entitiy's position based on target entity.
    m_Position.x = targetPos.x;
    m_Position.y = targetPos.y;
    m_Position.z = targetPos.z + 70.0f;
    m_Position += (-m_Camera.m_Forward * 80.0f);

    // Set this entity's camera data
    m_Camera.m_Pos = m_Position;
}

bool CFollowCamera::HandleMessage(const Telegram& telegram) {
    return false;
}

void CFollowCamera::SetTarget(BaseGameEntity* target) {
    assert(target != nullptr);
    m_Target       = target;
    m_Position     = target->m_Position;
    m_Camera.m_Pos = m_Position;
    printf("follow camera pos: %f, %f, %f\n", m_Position.x, m_Position.y, m_Position.z);
}
