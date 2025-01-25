//
// Created by benek on 10/14/24.
//

#include "g_enemy_states.h"

#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/gtx/vector_angle.hpp"

#include "../../Audio/Audio.h"
#include "../../Message/g_extra_info_types.h"
#include "../../Message/message_dispatcher.h"
#include "../../Message/message_type.h"
#include "../entity_manager.h"
#include "g_enemy.h"

static inline bool ReactToRayHit(Enemy* agent, Telegram telegram)
{
    BaseGameEntity* pSender = EntityManager::Instance()->GetEntityFromID(telegram.Sender);

    if ( pSender->Type() == ET_PLAYER )
    {
        double hitPoints = DereferenceToType<double>(telegram.ExtraInfo);
        agent->DecreaseHealth(hitPoints);
        if ( agent->IsDead() )
        {
            // Turn agent towards its shooter
            // TODO: This code is redundant as somewhere else something like this has to be done as well.
            // So make it a member function like 'LookAt' or something like that.
            glm::vec3 newForward          = pSender->m_Position - agent->m_Position;
            newForward.z                  = 0;
            newForward                    = glm::normalize(newForward);
            float     absOrientationAngle = glm::orientedAngle(DOD_WORLD_FORWARD, newForward, DOD_WORLD_UP);
            float     randAngleBias       = RandBetween(-30.0f, 30.0f);
            glm::quat newForwardOrientation
                = glm::angleAxis(absOrientationAngle + glm::radians(randAngleBias), DOD_WORLD_UP);
            agent->m_Orientation = newForwardOrientation;

            agent->GetFSM()->ChangeState(EnemyDead::Instance());
        }
        else
        {
            glm::vec3 pos = agent->m_Position;
            glm::vec3 vel = agent->m_Velocity;
            Audio::m_SfxBus.play3d(*agent->m_SfxHit, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
        }
    }

    return true;
}

static inline bool ReactToInViewMessage(Enemy* pAgent, InViewInfo inViewInfo)
{
    if ( inViewInfo.pOther->Type() == ET_PLAYER )
    {
        pAgent->SetTarget(inViewInfo.pOther);
        pAgent->GetFSM()->ChangeState(EnemyFollow::Instance());
    }

    return true;
}

EnemyIdle* EnemyIdle::Instance()
{
    static EnemyIdle instance;

    return &instance;
}

void EnemyIdle::Enter(Enemy* pEnemy)
{
    pEnemy->m_AnimationState = ANIM_STATE_IDLE;
    printf("Enemy entered Idle State\n");
}

void EnemyIdle::Execute(Enemy* pEnemy)
{
    //printf("Enemy is executing Idle State\n");
}

void EnemyIdle::Exit(Enemy* pEnemy)
{
    printf("Enemy is exiting Idle State\n");
}

bool EnemyIdle::OnMessage(Enemy* agent, const Telegram& telegram)
{
    //printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());

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

        agent->GetFSM()->ChangeState(EnemyAttacking::Instance());

        return true;
    }
    break;

    case message_type::EntityInView:
    {
        InViewInfo inViewInfo = DereferenceToType<InViewInfo>(telegram.ExtraInfo);
        return ReactToInViewMessage(agent, inViewInfo);
    }

    case message_type::RayHit:
    {
        return ReactToRayHit(agent, telegram);
    }
    break;

    case message_type::SetPatrol:
    {

        if ( agent->m_pPath != nullptr && !agent->m_Target.empty() )
        {

            agent->GetFSM()->ChangeState(EnemyPatrol::Instance());
            return true;
        }
        return false;
    }
    break;

    default:
    {
        return false;
    }
    }
}

EnemyAttacking* EnemyAttacking::Instance()
{
    static EnemyAttacking instance;

    return &instance;
}

void EnemyAttacking::Enter(Enemy* pEnemy)
{
    //printf("Enemy entered Attacking State\n");
    pEnemy->m_AnimationState = ANIM_STATE_ATTACK;
    pEnemy->m_AttackTimer    = pEnemy->m_AttackOffset;
}

void EnemyAttacking::Execute(Enemy* pEnemy)
{
    BaseGameEntity* pOther = pEnemy->m_pTargetEntity;
    if ( pOther == nullptr )
    {
        return;
    }

    // Check if still close enough to attack
    float     otherRadius = pOther->GetEllipsoidColliderPtr()->radiusA;
    float     selfRadius  = pEnemy->GetEllipsoidColliderPtr()->radiusA;
    glm::vec3 otherCenter = pOther->GetEllipsoidColliderPtr()->center;
    glm::vec3 selfCenter  = pEnemy->GetEllipsoidColliderPtr()->center;

    glm::vec3 selfToTarget = otherCenter - selfCenter;
    float     distance     = glm::length(selfToTarget);
    float     bias         = distance * 0.5f;
    if ( (distance - bias) < (otherRadius + selfRadius) ) // OK, close enough. Deal damage.
    {
        if ( pEnemy->m_AttackTimer > pEnemy->m_AttackRate )
        {
            pEnemy->m_AttackTimer = 0;
            glm::vec3 pos         = pEnemy->m_Position;
            glm::vec3 vel         = pEnemy->m_Velocity;
            auto handle = Audio::m_SfxBus.play3d(*pEnemy->m_SfxAttack, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
            Audio::m_Soloud.setRelativePlaySpeed(handle, 2.3f);
            Dispatcher->DispatchMessage(100.0, pEnemy->ID(), pOther->ID(), message_type::Hit, nullptr);
        }
        else
        {
            pEnemy->m_AttackTimer += GetDeltaTime();
        }
    }
    else // Other entity is too far away, follow it.
    {
        pEnemy->GetFSM()->ChangeState(EnemyFollow::Instance());
    }
}

void EnemyAttacking::Exit(Enemy* pEnemy)
{
    //printf("Player is exiting Attacking State\n");
}

bool EnemyAttacking::OnMessage(Enemy* agent, const Telegram& telegram)
{
    //printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());
    switch ( telegram.Message )
    {

    case message_type::RayHit:
    {
        return ReactToRayHit(agent, telegram);
    }
    break;

    default:
        return false;
    }
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

    // Make sure to turn steering off for dead bodies
    pEnemy->m_pSteeringBehaviour->SeekOff();
    pEnemy->m_pSteeringBehaviour->NoneOn();

    pEnemy->m_AnimationState = ANIM_STATE_DIE;
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
    // NOTE(Michael): Maybe do this.
    //return handleMessageAlive(agent, telegram);

    return false;
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
    pEnemy->m_AnimationState = ANIM_STATE_WALK;
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
    //printf("\nEnemy received telegram %s\n", MessageToString(telegram.Message).c_str());
    switch ( telegram.Message )
    {
    case message_type::EntityInView:
    {
        InViewInfo inViewInfo = DereferenceToType<InViewInfo>(telegram.ExtraInfo);
        return ReactToInViewMessage(agent, inViewInfo);
    }

    case message_type::RayHit:
    {
        return ReactToRayHit(agent, telegram);
    }
    break;

    default:
        return false;
    }

    return false;
}

// ---------------------------------
//
// Follow state
//
// ---------------------------------
EnemyFollow* EnemyFollow::Instance()
{
    static EnemyFollow instance;

    return &instance;
}

void EnemyFollow::Enter(Enemy* pEnemy)
{
    printf("Enemy entered Follow State\n");
    pEnemy->m_AnimationState = ANIM_STATE_RUN;
    pEnemy->m_pSteeringBehaviour->SetTargetAgent(pEnemy->m_pTargetEntity);
    pEnemy->m_pSteeringBehaviour->SeekOn();

    if ( !StateMachine<Enemy>::IsSameState(*pEnemy->GetFSM()->PreviousState(), *EnemyAttacking::Instance()) )
    { // only play if first spotted, not if player runs away after being attacked
        glm::vec3 pos = pEnemy->m_Position;
        glm::vec3 vel = pEnemy->m_Velocity;
        Audio::m_SfxBus.play3d(*pEnemy->m_SfxTargetSpotted, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
    }
    // pEnemy->m_pSteeringBehaviour->FollowWaypointsOn();
    //pEnemy->m_pSteeringBehaviour->FollowPathOn();
}

void EnemyFollow::Execute(Enemy* pEnemy) {}

void EnemyFollow::Exit(Enemy* pEnemy)
{
    printf("Player is exiting Follow State\n");
    //pEnemy->m_pSteeringBehaviour->FollowPathOff();
    //pEnemy->m_pSteeringBehaviour->FollowWaypointsOff();
}

bool EnemyFollow::OnMessage(Enemy* agent, const Telegram& telegram)
{
    //printf("\nEnemyFollow: Enemy received telegram %s\n", MessageToString(telegram.Message).c_str());
    switch ( telegram.Message )
    {
    case message_type::Collision:
    {
        BaseGameEntity* pSender = EntityManager::Instance()->GetEntityFromID(telegram.Sender);
        if ( pSender->Type() == ET_PLAYER )
        {
            //printf("Enemy hit player\n");
            agent->m_pSteeringBehaviour->SeekOff();
            agent->m_pSteeringBehaviour->NoneOn();
            agent->GetFSM()->ChangeState(EnemyAttacking::Instance());
            return true;
        }
        return false;
    }
    break;

    case message_type::RayHit:
    {
        ReactToRayHit(agent, telegram);
    }
    break;

    default:
        return false;
    }

    return false;
}
