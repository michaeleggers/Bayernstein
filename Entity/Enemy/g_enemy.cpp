//
// Created by benek on 10/14/24.
//

#include "g_enemy.h"
#include "g_enemy_states.h"
#include "../../input_handler.h"

#include <stdio.h>

void Enemy::Update() {
	m_pStateMachine->Update();
	//printf("Enemy Health: %f\n", m_Health);
}

Enemy::Enemy(const int id) : BaseGameEntity(id, ET_ENEMY), m_pStateMachine(nullptr) {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(EnemyIdle::Instance());
}
bool Enemy::HandleMessage(const Telegram &telegram) { 
	return m_pStateMachine->HandleMessage(telegram); 
}

void Enemy::HandleInput() {
    ButtonState captainState = CHECK_ACTION("set_captain");
    if ( captainState == ButtonState::WENT_DOWN ) {
        printf("Enemy: I am the captain!\n");
    }
}

