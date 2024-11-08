#include "g_follow_camera.h"

#include "../../camera.h"

CFollowCamera::CFollowCamera(const int id, glm::vec3 pos, BaseGameEntity* target)
    : BaseGameEntity(id, ET_CAMERA) {
	
	m_Position = pos;
	m_Target = target;
	m_Camera = Camera(pos);
}

CFollowCamera::~CFollowCamera() {
}

void CFollowCamera::Update() {
}

bool CFollowCamera::HandleMessage(const Telegram& telegram) {
}

