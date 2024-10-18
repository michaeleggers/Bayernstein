//
// Created by benek on 10/14/24.
//

#include "PlayerStates.h"

#include "../../utils.h"
#include "../Message/MessageType.h"
#include <iostream>

PlayerIdle *PlayerIdle::Instance() {
	static PlayerIdle instance;

	return &instance;
}

void PlayerIdle::Enter(Player *pPlayer) { std::cout << "Player entered Idle State\n"; }

void PlayerIdle::Execute(Player *pPlayer) { std::cout << "Player is executing Idle State\n"; }
void PlayerIdle::Exit(Player *pPlayer) { std::cout << "Player is exiting Idle State\n"; }
bool PlayerIdle::OnMessage(Player *agent, const Telegram &telegram) { return false; }
PlayerRunning *PlayerRunning::Instance() {
	static PlayerRunning instance;

	return &instance;
}

void PlayerRunning::Enter(Player *pPlayer) { std::cout << "Player entered Running State\n"; }

void PlayerRunning::Execute(Player *pPlayer) { std::cout << "Player is executing Running State\n"; }
void PlayerRunning::Exit(Player *pPlayer) { std::cout << "Player is exiting Running State\n"; }
bool PlayerRunning::OnMessage(Player *agent, const Telegram &telegram) { return false; }

PlayerAttacking *PlayerAttacking::Instance() {
	static PlayerAttacking instance;

	return &instance;
}

void PlayerAttacking::Enter(Player *pPlayer) { std::cout << "Player entered Running State\n"; }

void PlayerAttacking::Execute(Player *pPlayer) {
	if (pPlayer->CanAttack()) {
		BaseGameEntity *enemy =
			EntityManager::Instance()->GetEntityFromID(1); // NOTE: just for testing look for enemy with id 1

		double *value = new double(10.0);
		if (RandBetween(0.0, 1.0) < 0.9) {
			*value = 10.0;
			Dispatcher->DispatchMessage(0.0, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
		} else {
			*value = 50.0;
			Dispatcher->DispatchMessage(50.0, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
		}
	}
	std::cout << "Player is executing AttackingState\n";
}
void PlayerAttacking::Exit(Player *pPlayer) { std::cout << "Player is exiting Running State\n"; }
bool PlayerAttacking::OnMessage(Player *agent, const Telegram &telegram) { return false; }
