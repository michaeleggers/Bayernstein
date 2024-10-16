//
// Created by benek on 10/14/24.
//

#include "PlayerStates.h"
#include <iostream>


PlayerIdle *PlayerIdle::Instance() {
	static PlayerIdle instance;

	return &instance;
}

void PlayerIdle::Enter(Player *pPlayer) { std::cout << "Player entered Idle State\n"; }

void PlayerIdle::Execute(Player *pPlayer) { std::cout << "Player is executing Idle State\n"; }
void PlayerIdle::Exit(Player *pPlayer) { std::cout << "Player is exiting Idle State\n"; }

PlayerRunning *PlayerRunning::Instance() {
	static PlayerRunning instance;

	return &instance;
}

void PlayerRunning::Enter(Player *pPlayer) { std::cout << "Player entered Running State\n"; }

void PlayerRunning::Execute(Player *pPlayer) { std::cout << "Player is executing Running State\n"; }
void PlayerRunning::Exit(Player *pPlayer) { std::cout << "Player is exiting Running State\n"; }

PlayerAttacking *PlayerAttacking::Instance() {
	static PlayerAttacking instance;

	return &instance;
}

void PlayerAttacking::Enter(Player *pPlayer) { std::cout << "Player entered Running State\n"; }

void PlayerAttacking::Execute(Player *pPlayer) { std::cout << "Player is executing Running State\n"; }
void PlayerAttacking::Exit(Player *pPlayer) { std::cout << "Player is exiting Running State\n"; }
