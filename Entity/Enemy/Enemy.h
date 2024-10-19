//
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H
#include "../../FSM/StateMachine.h"
#include "../BaseGameEntity.h"

class Enemy : public BaseGameEntity {
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
