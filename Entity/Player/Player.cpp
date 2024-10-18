//
// Created by benek on 10/14/24.
//

#include "Player.h"

#include "../input.h"
#include "PlayerStates.h"
#include "iostream"
#include <SDL.h>

void Player::Update() {

	if (KeyPressed(SDLK_w)) {
		m_pStateMachine->ChangeState(PlayerRunning::Instance());
	}

	if (KeyPressed(SDLK_s)) {
		m_pStateMachine->ChangeState(PlayerIdle::Instance());
	}

	if (KeyPressed(SDLK_SPACE)) {
		m_pStateMachine->ChangeState(PlayerAttacking::Instance());
	}
	m_pStateMachine->Update();
}

Player::Player() : BaseGameEntity(0), m_pStateMachine(nullptr) {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(PlayerIdle::Instance());
}

bool Player::HandleMessage(const Telegram &telegram) { return m_pStateMachine->HandleMessage(telegram); }
