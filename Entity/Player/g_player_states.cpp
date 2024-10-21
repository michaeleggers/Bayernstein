//
// Created by benek on 10/14/24.
//

#include "g_player_states.h"

#include <stdio.h>
#include "../../utils.h"
#include "../Message/message_type.h"

PlayerIdle *PlayerIdle::Instance() {
	static PlayerIdle instance;

	return &instance;
}

void PlayerIdle::Enter(Player *pPlayer) {
	//printf("Player entered Idle State\n");
}

void PlayerIdle::Execute(Player *pPlayer) {
	//printf("Player is executing Idle State\n"); 
}

void PlayerIdle::Exit(Player *pPlayer) {
	//printf("Player is exiting Idle State\n"); 
}

bool PlayerIdle::OnMessage(Player *agent, const Telegram &telegram) { 
	return false; 
}

PlayerRunning *PlayerRunning::Instance() {
	static PlayerRunning instance;

	return &instance;
}

void PlayerRunning::Enter(Player *pPlayer) {
	//printf("Player entered Running State\n"); 
}

void PlayerRunning::Execute(Player *pPlayer) {
	//printf("Player is executing Running State\n"); 
}

void PlayerRunning::Exit(Player *pPlayer) {
	//printf("Player is exiting Running State\n"); 
}

bool PlayerRunning::OnMessage(Player *agent, const Telegram &telegram) {
	return false; 
}

PlayerAttacking *PlayerAttacking::Instance() {
	static PlayerAttacking instance;

	return &instance;
}

void PlayerAttacking::Enter(Player *pPlayer) {
	//printf("Player entered Running State\n"); 
}

void PlayerAttacking::Execute(Player *pPlayer) {
	if (pPlayer->CanAttack()) {
		BaseGameEntity *enemy =
			EntityManager::Instance()->GetEntityFromID(1); // NOTE: just for testing look for enemy with id 1

		double *value = new double(10.0);
		if (RandBetween(0.0, 1.0) < 0.9) {
			*value = 10.0;
			Dispatcher->DispatchMessage(SEND_MSG_IMMEDIATELY, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
		} else {
			*value = 50.0;
			Dispatcher->DispatchMessage(50.0, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
		}
	}
	//printf("Player is executing AttackingState\n");
}

void PlayerAttacking::Exit(Player *pPlayer) {
	//printf("Player is exiting Running State\n"); 
}

bool PlayerAttacking::OnMessage(Player *agent, const Telegram &telegram) { 
	return false; 
}

