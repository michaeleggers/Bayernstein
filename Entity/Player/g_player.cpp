//
// Created by benek on 10/14/24.
//

#include "g_player.h"

#include "../../irender.h"
#include "../input.h"
#include "g_player_states.h"

#include <SDL.h>

void Player::Update(double dt) {

	if (KeyPressed(SDLK_w)) {
		m_pStateMachine->ChangeState(PlayerRunning::Instance());
	} else if (KeyPressed(SDLK_SPACE)) {
		m_pStateMachine->ChangeState(PlayerAttacking::Instance());
	} else {
		m_pStateMachine->ChangeState(PlayerIdle::Instance());
	}

	UpdatePlayerModel(dt);
	m_pStateMachine->Update();
}

Player::Player(glm::vec3 initialPosition)
	: BaseGameEntity(0), m_Forward({ 0.0, -1.0, 0.0 }), m_Side({ -1.0, 0.0, 0.0 }), m_RotationAngle(0.0),
	  m_Velocity(0.0), m_AnimationState(ANIM_STATE_IDLE), m_CollisionInfo() {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(PlayerIdle::Instance());
	LoadModel("models/multiple_anims/multiple_anims.iqm", initialPosition);
}

void Player::LoadModel(const char* path, glm::vec3 initialPosition) {
	// Load IQM Model
	IQMModel iqmModel = LoadIQM(path);

	// Convert the model to our internal format
	m_Model = CreateModelFromIQM(&iqmModel);
	m_Model.isRigidBody = false;
	m_Model.position = initialPosition;
	m_Model.scale = glm::vec3(22.0f);

	for (int i = 0; i < m_Model.animations.size(); i++) {
		EllipsoidCollider* ec = &m_Model.ellipsoidColliders[i];
		ec->radiusA *= m_Model.scale.x;
		ec->radiusB *= m_Model.scale.z;
		ec->center = m_Model.position + glm::vec3(0.0f, 0.0f, ec->radiusB);
		glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
		ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
	}

	SetAnimState(&m_Model, ANIM_STATE_WALK);
}

void Player::UpdateCamera(Camera* camera) {

	// Fix camera position
	camera->m_Pos.x = m_Model.position.x;
	camera->m_Pos.y = m_Model.position.y;
	camera->m_Pos.z = m_Model.position.z + 70.0f;
	camera->m_Pos += (-m_Forward * 80.0f);
	// m_RotationAngle should already have the information about if we want to move left or right
	if (KeyPressed(SDLK_RIGHT) || KeyPressed(SDLK_LEFT)) {
		camera->RotateAroundUp(m_RotationAngle);
	}
}
void Player::Render(IRender* renderer, Camera* camera) {
	HKD_Model* models[1] = { &m_Model };
	renderer->SetActiveCamera(camera);
	renderer->Render(camera, models, 1);

	// Draw Debug Line for player veloctiy vector
	EllipsoidCollider ec = m_Model.ellipsoidColliders[m_Model.currentAnimIdx];
	Line velocityDebugLine = { Vertex(ec.center), Vertex(ec.center + 200.0f * m_Velocity) };
	velocityDebugLine.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	velocityDebugLine.b.color = velocityDebugLine.a.color;
	renderer->ImDrawLines(velocityDebugLine.vertices, 2, false);

	// Draw Debug Collision Collider
	if (m_CollisionInfo.didCollide) {
		m_Model.debugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
		renderer->ImDrawSphere(m_CollisionInfo.hitPoint, 5.0f);
		// printf("hitpoint: %f, %f, %f\n", collisionInfo.hitPoint.x, collisionInfo.hitPoint.y,
		// collisionInfo.hitPoint.z);
	} else {
		m_Model.debugColor = glm::vec4(1.0f); // white
	}

	renderer->RenderColliders(camera, models, 1);
}

void Player::UpdatePlayerModel(double dt) {
	float followCamSpeed = 0.5f;
	float followTurnSpeed = 0.2f;
	if (KeyPressed(SDLK_LSHIFT)) {
		followCamSpeed *= 0.3f;
		followTurnSpeed *= 0.3f;
	}

	// Model rotation

	m_RotationAngle = followTurnSpeed * (float)dt;
	if (KeyPressed(SDLK_RIGHT)) {
		m_RotationAngle = -m_RotationAngle;
		glm::quat rot = glm::angleAxis(glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Model.orientation *= rot;
	}
	if (KeyPressed(SDLK_LEFT)) {
		glm::quat rot = glm::angleAxis(glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Model.orientation *= rot;
	}

	m_Forward = glm::rotate(m_Model.orientation,
							glm::vec3(0.0f, -1.0f, 0.0f)); // -1 because the model is facing -1 (Outside the screen)
	m_Side = glm::cross(m_Forward, glm::vec3(0.0f, 0.0f, 1.0f));

	// Change player's velocity and animation state based on input
	m_Velocity = glm::vec3(0.0f);
	float t = (float)dt * followCamSpeed;
	AnimState playerAnimState = ANIM_STATE_IDLE;
	if (KeyPressed(SDLK_w)) {
		m_Velocity += t * m_Forward;
		playerAnimState = ANIM_STATE_RUN;
	}
	if (KeyPressed(SDLK_s)) {
		m_Velocity -= t * m_Forward;
		playerAnimState = ANIM_STATE_RUN;
	}
	if (KeyPressed(SDLK_d)) {
		m_Velocity += t * m_Side;
		playerAnimState = ANIM_STATE_RUN;
	}
	if (KeyPressed(SDLK_a)) {
		m_Velocity -= t * m_Side;
		playerAnimState = ANIM_STATE_RUN;
	}

	if (playerAnimState == ANIM_STATE_RUN) {
		if (KeyPressed(SDLK_LSHIFT)) {
			playerAnimState = ANIM_STATE_WALK;
		}
	}

	SetAnimState(&m_Model, playerAnimState);

	// Test collision between player and world geometry
	EllipsoidCollider ec = m_Model.ellipsoidColliders[m_Model.currentAnimIdx];
	m_CollisionInfo = CollideEllipsoidWithTriPlane(ec,
												   m_Velocity,
												   static_cast<float>(dt) * m_World->m_Gravity,
												   m_World->m_TriPlanes.data(),
												   m_World->m_TriPlanes.size());

	// Update the ellipsoid colliders for all animation states based on the new collision position
	for (int i = 0; i < m_Model.animations.size(); i++) {
		m_Model.ellipsoidColliders[i].center = m_CollisionInfo.basePos;
	}
	m_Model.position.x = m_CollisionInfo.basePos.x;
	m_Model.position.y = m_CollisionInfo.basePos.y;
	m_Model.position.z = m_CollisionInfo.basePos.z - ec.radiusB;

	UpdateModel(&m_Model, (float)dt);
}

bool Player::HandleMessage(const Telegram& telegram) {
	return m_pStateMachine->HandleMessage(telegram);
}
