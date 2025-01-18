//
// Created by benek on 10/14/24.
//

#include "g_enemy_states.h"
#include "../../Audio/Audio.h"
#include "../../Message/message_type.h"
#include "g_enemy.h"
#include <stdio.h>

bool handleMessageAlive(Enemy* agent, const Telegram& telegram)
{
    printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());

    switch ( telegram.Message )
    {
    case message_type::Attack:
    {
        agent->DecreaseHealth(*(double*)telegram.ExtraInfo * 2.0);
        if ( agent->IsDead() )
        {
            agent->GetFSM()->ChangeState(EnemyDead::Instance());
            return true;
        }

        if (!agent->GetFSM()->IsInState(*EnemyAttacking::Instance())) {
            agent->GetFSM()->ChangeState(EnemyAttacking::Instance());
        }

        return true;
    }
    break;

    case message_type::RayHit:
    {
        agent->DecreaseHealth(20.0f);
        if ( agent->IsDead() )
        {
            agent->GetFSM()->ChangeState(EnemyDead::Instance());
            return true;
        }
        else
        {
            glm::vec3 pos = agent->m_Position;
            glm::vec3 vel = agent->m_Velocity;
            Audio::m_SfxBus.play3d(*agent->m_SfxHit, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
        }

        return true;
    }
    break;

    default:
    {
        return false;
    }
    }
}

EnemyIdle* EnemyIdle::Instance()
{
    static EnemyIdle instance;

    return &instance;
}

void EnemyIdle::Enter(Enemy* pEnemy)
{
    //printf("Enemy entered Idle State\n");
}

void EnemyIdle::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Idle State\n");
}

void EnemyIdle::Exit(Enemy* pEnemy)
{
    //printf("Enemy is exiting Idle State\n");
}

bool EnemyIdle::OnMessage(Enemy* agent, const Telegram& telegram)
{
    return handleMessageAlive(agent, telegram);
}

EnemyAttacking* EnemyAttacking::Instance()
{
    static EnemyAttacking instance;

    return &instance;
}

void EnemyAttacking::Enter(Enemy* pEnemy)
{
    //printf("Enemy entered Attacking State\n");
}

void EnemyAttacking::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Attacking State\n");
}

void EnemyAttacking::Exit(Enemy* pEnemy)
{
    //printf("Player is exiting Attacking State\n");
}

bool EnemyAttacking::OnMessage(Enemy* agent, const Telegram& telegram)
{
    return handleMessageAlive(agent, telegram);
}
EnemyDead* EnemyDead::Instance()
{
    static EnemyDead instance;

    return &instance;
}

void EnemyDead::Enter(Enemy* pEnemy)
{
    glm::vec3 pos = pEnemy->m_Position;
    glm::vec3 vel = pEnemy->m_Velocity;
    Audio::m_SfxBus.play3d(*pEnemy->m_SfxDeath, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
    printf("Enemy entered Dead State\n");
}

void EnemyDead::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Dead State\n");
}

void EnemyDead::Exit(Enemy* pEnemy)
{
    //printf("Player is exiting Dead State\n");
}

bool EnemyDead::OnMessage(Enemy* agent, const Telegram& telegram)
{
    return false;
}

// ---------------------------------
//
// Wander State
//
// ---------------------------------
EnemyWander* EnemyWander::Instance()
{
    static EnemyWander instance;

    return &instance;
}

void EnemyWander::Enter(Enemy* pEnemy)
{
    printf("Enemy entered Wander State\n");
    pEnemy->m_pSteeringBehaviour->WanderOn();
}

void EnemyWander::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Wander State\n");
}

void EnemyWander::Exit(Enemy* pEnemy)
{
    printf("Player is exiting Wander State\n");
    pEnemy->m_pSteeringBehaviour->WanderOff();
}

bool EnemyWander::OnMessage(Enemy* agent, const Telegram& telegram)
{
    return handleMessageAlive(agent, telegram);
}

// ---------------------------------
//
// Patrol State
//
// ---------------------------------
EnemyPatrol* EnemyPatrol::Instance()
{
    static EnemyPatrol instance;

    return &instance;
}

void EnemyPatrol::Enter(Enemy* pEnemy)
{
    printf("Enemy entered Patrol State\n");
    // pEnemy->m_pSteeringBehaviour->FollowWaypointsOn();
    pEnemy->m_pSteeringBehaviour->FollowPathOn();
}

void EnemyPatrol::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Patrol State\n");
}

void EnemyPatrol::Exit(Enemy* pEnemy)
{
    printf("Player is exiting Patrol State\n");
    pEnemy->m_pSteeringBehaviour->FollowPathOff();
    pEnemy->m_pSteeringBehaviour->FollowWaypointsOff();
}

bool EnemyPatrol::OnMessage(Enemy* agent, const Telegram& telegram)
{
    return handleMessageAlive(agent, telegram);
}
