//
// Created by benek on 10/14/24.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../BaseGameEntity.h"
#include "../FSM/StateMachine.h"
#include "../Message/MessageDispatcher.h"

class Player : public BaseGameEntity {
  private:
	StateMachine<Player> *m_pStateMachine;

  public:
	void Update() override;
	explicit Player();

	~Player() override { delete m_pStateMachine; }
	[[nodiscard]] StateMachine<Player> *GetFSM() const { return m_pStateMachine; }

	bool HandleMessage(const Telegram &message) override;
};

#endif // PLAYER_H
