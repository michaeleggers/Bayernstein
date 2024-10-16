//
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H
#include "../BaseGameEntity.h"
#include "../FSM/StateMachine.h"

class Enemy : public BaseGameEntity {
  private:
	StateMachine<Enemy> *m_pStateMachine;

  public:
	void Update() override;
	explicit Enemy(int id);

	~Enemy() override { delete m_pStateMachine; }
	[[nodiscard]] StateMachine<Enemy> *GetFSM() const { return m_pStateMachine; }

	bool HandleMessage(const Telegram &message) override;
};

#endif // ENEMY_H
