//
// Created by benek on 10/14/24.
//

#include "g_enemy_states.h"
#include <stdio.h>
#include "../../Message/message_type.h"

EnemyIdle *EnemyIdle::Instance() {
    static EnemyIdle instance;

    return &instance;
}

void EnemyIdle::Enter(Enemy *pEnemy) { 
    //printf("Enemy entered Idle State\n"); 
}

void EnemyIdle::Execute(Enemy *pEnemy) { 
    //printf("Enemy is executing Idle State\n"); 
}

void EnemyIdle::Exit(Enemy *pEnemy) {
    //printf("Enemy is exiting Idle State\n"); 
}

bool EnemyIdle::OnMessage(Enemy *agent, const Telegram &telegram) {
    //printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());

    switch (telegram.Message) {
        case message_type::Attack: {
            agent->DecreaseHealth(*(double *)telegram.ExtraInfo * 2.0);
            if (agent->IsDead()) {
                agent->GetFSM()->ChangeState(EnemyDead::Instance());
                return true;
            }

            agent->GetFSM()->ChangeState(EnemyAttacking::Instance());

            return true;
        } break;
        default: {
            return false;
        }
    }
}

EnemyRunning *EnemyRunning::Instance() {
    static EnemyRunning instance;

    return &instance;
}

void EnemyRunning::Enter(Enemy *pEnemy) { 
    //printf("Enemy entered Running State\n"); 
}

void EnemyRunning::Execute(Enemy *pEnemy) { 
    //printf("Enemy is executing Running State\n"); 
}

void EnemyRunning::Exit(Enemy *pEnemy) { 
    //printf("Player is exiting Running State\n"); 
}

bool EnemyRunning::OnMessage(Enemy *agent, const Telegram &telegram) { 
    return false; 
}

EnemyAttacking *EnemyAttacking::Instance() {
    static EnemyAttacking instance;

    return &instance;
}

void EnemyAttacking::Enter(Enemy *pEnemy) { 
    //printf("Enemy entered Attacking State\n"); 
}

void EnemyAttacking::Execute(Enemy *pEnemy) { 
    //printf("Enemy is executing Attacking State\n"); 
}

void EnemyAttacking::Exit(Enemy *pEnemy) { 
    //printf("Player is exiting Attacking State\n"); 
}

bool EnemyAttacking::OnMessage(Enemy *agent, const Telegram &telegram) {
    printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());
    switch (telegram.Message) {
    case message_type::Attack: {
        agent->DecreaseHealth(*(double *)telegram.ExtraInfo);

        if (agent->IsDead()) {
            agent->GetFSM()->ChangeState(EnemyDead::Instance());
            return true;
        }

        return true;
    }
    default:
        return false;
    }
}
EnemyDead *EnemyDead::Instance() {
    static EnemyDead instance;

    return &instance;
}

void EnemyDead::Enter(Enemy *pEnemy) { 
    //printf("Enemy entered Dead State\n"); 
}

void EnemyDead::Execute(Enemy *pEnemy) { 
    //printf("Enemy is executing Dead State\n"); 
}

void EnemyDead::Exit(Enemy *pEnemy) { 
    //printf("Player is exiting Dead State\n"); 
}

bool EnemyDead::OnMessage(Enemy *agent, const Telegram &telegram) { 
    return false; 
}

