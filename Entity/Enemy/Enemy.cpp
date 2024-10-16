//
// Created by benek on 10/14/24.
//

#include "Enemy.h"
#include "EnemyStates.h"
#include <iostream>

void Enemy::Update() {}

Enemy::Enemy(const int id) : BaseGameEntity(id), m_pStateMachine(nullptr) {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(EnemyIdle::Instance());
}
bool Enemy::HandleMessage(const Telegram &message) {
	std::cout << "Message Recieved" << message << std::endl;
	return true;
}
