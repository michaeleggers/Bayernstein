#include "g_fly_camera.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

CFlyCamera::CFlyCamera(const int id, glm::vec3 pos)
	: BaseGameEntity(id, ET_FLY_CAMERA) {
}

CFlyCamera::~CFlyCamera() {
}

void CFlyCamera::Update() {
}

bool CFlyCamera::HandleMessage(const Telegram& telegram) {
	return false;
}

void CFlyCamera::HandleInput() {
}

