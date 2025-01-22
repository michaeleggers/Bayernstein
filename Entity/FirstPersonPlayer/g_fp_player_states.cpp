//
// Created by benek on 10/14/24.
//

#include "g_fp_player_states.h"
#include "g_fp_player.h"

#include "../../utils/utils.h"
#include "../Message/message_type.h"
#include <stdio.h>

FirstPersonPlayerIdle* FirstPersonPlayerIdle::Instance() {
    static FirstPersonPlayerIdle instance;

    return &instance;
}

void FirstPersonPlayerIdle::Enter(FirstPersonPlayer* pPlayer) {
    //printf("Player entered Idle State\n");
}

void FirstPersonPlayerIdle::Execute(FirstPersonPlayer* pPlayer) {
    //printf("Player is executing Idle State\n");
}

void FirstPersonPlayerIdle::Exit(FirstPersonPlayer* pPlayer) {
    //printf("Player is exiting Idle State\n");
}

bool FirstPersonPlayerIdle::OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) {
    switch ( telegram.Message )
    {

    case message_type::Hit:
    {
        BaseGameEntity* pSender = EntityManager::Instance()->GetEntityFromID(telegram.Sender);
        //printf("Hit by enemy!\n");
        // TODO: Implement.
        // - before enemy sends this message it must check if its cooldown is to 0.
        // otherwise it will spam this message.
        return true;
    }
    break;

    default:
        return false;
    }
    
}

FirstPersonPlayerRunning* FirstPersonPlayerRunning::Instance() {
    static FirstPersonPlayerRunning instance;

    return &instance;
}

void FirstPersonPlayerRunning::Enter(FirstPersonPlayer* pPlayer) {
    //printf("Player entered Running State\n");
}

void FirstPersonPlayerRunning::Execute(FirstPersonPlayer* pPlayer) {
    //printf("Player is executing Running State\n");
}

void FirstPersonPlayerRunning::Exit(FirstPersonPlayer* pPlayer) {
    //printf("Player is exiting Running State\n");
}

bool FirstPersonPlayerRunning::OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) {
    return false;
}

FirstPersonPlayerAttacking* FirstPersonPlayerAttacking::Instance() {
    static FirstPersonPlayerAttacking instance;

    return &instance;
}

void FirstPersonPlayerAttacking::Enter(FirstPersonPlayer* pPlayer) {
    //printf("Player entered Running State\n");
}

void FirstPersonPlayerAttacking::Execute(FirstPersonPlayer* pPlayer) {
    if ( pPlayer->CanAttack() ) {
        BaseGameEntity* enemy
            = EntityManager::Instance()->GetEntityFromID(1); // NOTE: just for testing look for enemy with id 1

        double* value = new double(10.0);
        if ( RandBetween(0.0, 1.0) < 0.9 ) {
            *value = 10.0;
            Dispatcher->DispatchMessage(SEND_MSG_IMMEDIATELY, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
        } else {
            *value = 50.0;
            Dispatcher->DispatchMessage(50.0, pPlayer->ID(), enemy->ID(), message_type::Attack, value);
        }
    }
    //printf("Player is executing AttackingState\n");
}

void FirstPersonPlayerAttacking::Exit(FirstPersonPlayer* pPlayer) {
    //printf("Player is exiting Running State\n");
}

bool FirstPersonPlayerAttacking::OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) {
    return false;
}
