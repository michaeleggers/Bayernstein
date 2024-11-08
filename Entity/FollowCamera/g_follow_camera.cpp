#include "g_follow_camera.h"

#include <assert.h>

#include "../../camera.h"

CFollowCamera::CFollowCamera(const int id, BaseGameEntity* target)
    : BaseGameEntity(id, ET_CAMERA) {

	assert( target != nullptr );

	m_Target = target;
	m_Position = target->m_Position;
	m_Camera = Camera(m_Position);
}

CFollowCamera::~CFollowCamera() {
}

void CFollowCamera::Update() {
	glm::vec3 targetPos = m_Target->m_Position;

	// Set this entitiy's position based on target entity.
    m_Position.x = targetPos.x;
    m_Position.y = targetPos.y;
    m_Position.z = targetPos.z + 70.0f;
    m_Position += (-m_Camera.m_Forward * 80.0f);

	// Set this entity's camera data
	m_Camera.m_Pos = m_Position;
}

bool CFollowCamera::HandleMessage(const Telegram& telegram) {
}

void CFollowCamera::SetTarget(BaseGameEntity* target) {
	assert( target != nullptr );
	m_Target = target;
	m_Position = target->m_Position;
	m_Camera.m_Pos = m_Position;
	printf("follow camera pos: %f, %f, %f\n", m_Position.x, m_Position.y, m_Position.z);
}

