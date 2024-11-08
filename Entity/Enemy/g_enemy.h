//
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H
#include "../../FSM/state_machine.h"
#include "../base_game_entity.h"
#include "../../input_receiver.h"

// TODO: Making the enemy controllable for testing input system.
class Enemy : public BaseGameEntity, public IInputReceiver {
  private:
	StateMachine<Enemy>* m_pStateMachine;
	double m_Health = 100;

  public:
	void Update() override;
	explicit Enemy(int id);

	~Enemy() override {
		delete m_pStateMachine;
	}
	[[nodiscard]] StateMachine<Enemy>* GetFSM() const {
		return m_pStateMachine;
	}

	bool HandleMessage(const Telegram& message) override;
	void HandleInput() override;

  public:
	bool DecreaseHealth(double amount) {
		m_Health = m_Health - amount;
		return true;
	}

	bool IsDead() {
		return m_Health <= 0.0;
	}
};

#endif // ENEMY_H
