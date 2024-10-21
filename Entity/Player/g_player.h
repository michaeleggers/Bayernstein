//
// Created by benek on 10/14/24.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../base_game_entity.h"

class Player : public BaseGameEntity {
  private:
	StateMachine<Player>* m_pStateMachine;
	double m_AttackDelay = 100;
	double m_LastAttack = 0;

  public:
	void Update() override;
	explicit Player();

	~Player() override {
		delete m_pStateMachine;
	}
	[[nodiscard]] StateMachine<Player>* GetFSM() const {
		return m_pStateMachine;
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
