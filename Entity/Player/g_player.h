//
// Created by benek on 10/14/24.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../irender.h"
#include "../../r_model.h"
#include "../base_game_entity.h"

class Player : public BaseGameEntity {
  private:
	StateMachine<Player>* m_pStateMachine;
	HKD_Model m_Model;
	// attacking members
  private:
	double m_AttackDelay = 100;
	double m_LastAttack = 0;
	// moving members
  private:
	glm::vec3 m_Forward, m_Side;
	float m_RotationAngle;
	glm::vec3 m_Velocity;
	AnimState m_AnimationState;
	CollisionInfo m_CollisionInfo;

	void LoadModel(const char* path, glm::vec3 initialPosition);
	void UpdatePlayerModel(double dt);

  public:
	void Update(double dt) override;
	void UpdateCamera(Camera* camera);
	void Render(IRender* renderer, Camera* camera);
	explicit Player(glm::vec3 initialPosition);

	~Player() override {
		delete m_pStateMachine;
	}
	[[nodiscard]] StateMachine<Player>* GetFSM() const {
		return m_pStateMachine;
	}

	HKD_Model* GetModel() {
		return &m_Model;
	}

	bool HandleMessage(const Telegram& telegram) override;

  public:
	bool CanAttack() {
		double currentTime = Clock->GetTime();
		if (currentTime >= m_LastAttack + m_AttackDelay) {
			m_LastAttack = currentTime;
			return true;
		}
		return false;
	}
};

#endif // PLAYER_H
