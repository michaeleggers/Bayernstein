//
// Created by benek on 10/14/24.
//

#include "g_enemy_states.h"
#include "../../Message/message_type.h"
#include "g_enemy.h"
#include <stdio.h>

EnemyIdle* EnemyIdle::Instance() {
    static EnemyIdle instance;

    return &instance;
}

void EnemyIdle::Enter(Enemy* pEnemy) {
    //printf("Enemy entered Idle State\n");
}

void EnemyIdle::Execute(Enemy* pEnemy) {
    //printf("Enemy is executing Idle State\n");
}

void EnemyIdle::Exit(Enemy* pEnemy) {
    //printf("Enemy is exiting Idle State\n");
}

bool EnemyIdle::OnMessage(Enemy* agent, const Telegram& telegram) {
    //printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());

    switch ( telegram.Message ) {
    case message_type::Attack: {
        agent->DecreaseHealth(*(double*)telegram.ExtraInfo * 2.0);
        if ( agent->IsDead() ) {
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

EnemyAttacking* EnemyAttacking::Instance() {
    static EnemyAttacking instance;

    return &instance;
}

void EnemyAttacking::Enter(Enemy* pEnemy) {
    //printf("Enemy entered Attacking State\n");
}

void EnemyAttacking::Execute(Enemy* pEnemy) {
    //printf("Enemy is executing Attacking State\n");
}

void EnemyAttacking::Exit(Enemy* pEnemy) {
    //printf("Player is exiting Attacking State\n");
}

bool EnemyAttacking::OnMessage(Enemy* agent, const Telegram& telegram) {
    printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());
    switch ( telegram.Message ) {
    case message_type::Attack: {
        agent->DecreaseHealth(*(double*)telegram.ExtraInfo);

        if ( agent->IsDead() ) {
            agent->GetFSM()->ChangeState(EnemyDead::Instance());
            return true;
        }

        return true;
    }
    default:
        return false;
    }
}
EnemyDead* EnemyDead::Instance() {
    static EnemyDead instance;

    return &instance;
}

void EnemyDead::Enter(Enemy* pEnemy) {
    //printf("Enemy entered Dead State\n");
}

void EnemyDead::Execute(Enemy* pEnemy) {
    //printf("Enemy is executing Dead State\n");
}

void EnemyDead::Exit(Enemy* pEnemy) {
    //printf("Player is exiting Dead State\n");
}

bool EnemyDead::OnMessage(Enemy* agent, const Telegram& telegram) {
    return false;
}

// ---------------------------------
//
// Wander State
//
// ---------------------------------
EnemyWander* EnemyWander::Instance() {
    static EnemyWander instance;

    return &instance;
}

void EnemyWander::Enter(Enemy* pEnemy) {
    printf("Enemy entered Wander State\n");
    pEnemy->m_pSteeringBehaviour->WanderOn();
}

void EnemyWander::Execute(Enemy* pEnemy) {
    //printf("Enemy is executing Wander State\n");
}

void EnemyWander::Exit(Enemy* pEnemy) {
    printf("Player is exiting Wander State\n");
    pEnemy->m_pSteeringBehaviour->WanderOff();
}

bool EnemyWander::OnMessage(Enemy* agent, const Telegram& telegram) {
    return false;
}

// ---------------------------------
//
// Patrol State
//
// ---------------------------------
EnemyPatrol* EnemyPatrol::Instance() {
    static EnemyPatrol instance;

    return &instance;
}

void EnemyPatrol::Enter(Enemy* pEnemy) {
    printf("Enemy entered Patrol State\n");
    // pEnemy->m_pSteeringBehaviour->FollowWaypointsOn();
    pEnemy->m_pSteeringBehaviour->FollowPathOn();
}

void EnemyPatrol::Execute(Enemy* pEnemy) {
    //printf("Enemy is executing Patrol State\n");
}

void EnemyPatrol::Exit(Enemy* pEnemy) {
    printf("Player is exiting Patrol State\n");
    pEnemy->m_pSteeringBehaviour->FollowPathOff();
    pEnemy->m_pSteeringBehaviour->FollowWaypointsOff();
}

bool EnemyPatrol::OnMessage(Enemy* agent, const Telegram& telegram) {
    return false;
}
