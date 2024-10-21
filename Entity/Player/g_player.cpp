//
// Created by benek on 10/14/24.
//

#include "g_player.h"

#include "../input.h"
#include "g_player_states.h"
#include <SDL.h>

void Player::Update() {

	if (KeyPressed(SDLK_w)) {
		m_pStateMachine->ChangeState(PlayerRunning::Instance());
	} else if (KeyPressed(SDLK_SPACE)) {
		m_pStateMachine->ChangeState(PlayerAttacking::Instance());
	} else {
		m_pStateMachine->ChangeState(PlayerIdle::Instance());
	}

	m_pStateMachine->Update();
}

Player::Player(const int id) : BaseGameEntity(id), m_pStateMachine(nullptr) {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(PlayerIdle::Instance());
}

bool Player::HandleMessage(const Telegram& telegram) {
	return m_pStateMachine->HandleMessage(telegram);
}
