//enem
// Created by benek on 10/14/24.
//

#include "g_enemy.h"

#include <SDL.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/gtx/vector_angle.hpp"

#include "../../Audio/Audio.h"
#include "../../globals.h"
#include "../../input.h"
#include "../../input_handler.h"
#include "../../utils/quick_math.h"
#include "../../utils/utils.h"
#include "g_enemy_states.h"

Enemy::Enemy(const std::vector<Property>& properties)
    : MovingEntity(ET_ENEMY),
      m_AnimationState(ANIM_STATE_IDLE)
{

    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(EnemyIdle::Instance());

    // We want position and, if applicable, a target.
    BaseGameEntity::GetProperty<glm::vec3>(properties, "origin", &m_Position);
    // FIX: Mem Leak on exit if target is not being set.
    BaseGameEntity::GetProperty<std::string>(properties, "target", &m_Target);
    int angle = 0;
    BaseGameEntity::GetProperty<int>(properties, "angle", &angle);

    //LoadModel("models/multiple_anims/multiple_anims.iqm", m_Position);
    LoadModel("models/mayan_undead_warrior/mayan_undead_warrior_final.iqm", m_Position);
    m_Model.pOwner       = this;
    m_Velocity           = glm::vec3(0.0f, 0.0f, 0.0f);
    m_PrevPosition       = GetEllipsoidColliderPtr()->center;
    m_Position           = m_PrevPosition;
    m_pSteeringBehaviour = new SteeringBehaviour(this);
    m_pPath              = nullptr;
    // m_pSteeringBehaviour->WanderOn();

    // Set viewing frustum
    m_ProjDistance = 5.0f;
    m_AspectRatio  = 3.35f;
    m_Near         = 0.1f;
    m_Far          = 500.0f;

    // Set initial orientation in the world
    // HACK: -90 degrees because some mismatch between quake editor and stuff...
    m_Orientation = glm::angleAxis(glm::radians((float)angle - 90.0f), DOD_WORLD_UP);
    m_Forward     = glm::rotate(m_Orientation, DOD_WORLD_FORWARD);

    m_SfxFootsteps
        = Audio::LoadSource("sfx/sonniss/015_Foley_Footsteps_Asphalt_Boot_Walk_Fast_Run_Jog_Close.wav", 1.0f, true);
    m_SfxHit           = Audio::LoadSource("sfx/sonniss/Creature_Monster_Attack_09.wav");
    m_SfxDeath         = Audio::LoadSource("sfx/sonniss/Creature_Alien_Death_01.wav");
    m_SfxTargetSpotted = Audio::LoadSource("sfx/sonniss/BEAST 01 Attack 08.wav", 0.6);
}

void Enemy::PreCollisionUpdate()
{
    float     dt           = (float)GetDeltaTime();
    glm::vec3 force        = m_pSteeringBehaviour->Calculate();
    glm::vec3 acceleration = force / m_Mass;

    // NOTE: There is no dynamic movement happening as it didn't
    // play nicely with slopes and it was hard to control
    // the speed behaviour.
    //
    //update velocity
    //m_Velocity += acceleration;
    //m_Velocity = math::TruncateVec3(m_Velocity, m_MaxSpeed);

    m_Velocity = force;
    if ( Speed() > 0.001 )
    {
        // Calculate the new forward direction
        glm::vec3 newForward = glm::normalize(m_Velocity);

        // Apply the rotation to the current orientation
        float     absOrientationAngle   = glm::orientedAngle(DOD_WORLD_FORWARD, newForward, m_Up);
        glm::quat newForwardOrientation = glm::angleAxis(absOrientationAngle, m_Up);
        m_Orientation                   = newForwardOrientation;

        // Update the forward and side vectors
        m_Forward = newForward;
        m_Side    = glm::cross(m_Forward, m_Up);

        if ( Audio::m_Soloud.isValidVoiceHandle(m_FootstepsHandle) )
        {
            Audio::m_Soloud.set3dSourcePosition(m_FootstepsHandle, m_Position.x, m_Position.y, m_Position.z);
            Audio::m_Soloud.set3dSourceVelocity(m_FootstepsHandle, m_Velocity.x, m_Velocity.y, m_Velocity.z);
            Audio::m_Soloud.update3dAudio();
        }
        else
        {
            m_FootstepsHandle = Audio::m_SfxBus.play3d(
                *m_SfxFootsteps, m_Position.x, m_Position.y, m_Position.z, m_Velocity.x, m_Velocity.y, m_Velocity.z);
        }
    }

    if ( Speed() >= 0.00001f )
    {
        //m_AnimationState = ANIM_STATE_WALK;
        Audio::m_Soloud.setRelativePlaySpeed(m_FootstepsHandle, 0.6f);
        Audio::m_Soloud.setVolume(m_FootstepsHandle, m_SfxFootsteps->mVolume * 0.4f);
        if ( Speed() > m_MaxSpeed * 0.5f )
        { // FIXME: will this condition ever happen
            //m_AnimationState = ANIM_STATE_RUN;
            Audio::m_Soloud.setRelativePlaySpeed(m_FootstepsHandle,
                                                 0.9f); // adjust sample speed to better match animation
            Audio::m_Soloud.setVolume(m_FootstepsHandle, m_SfxFootsteps->mVolume);
        }
    }
    else
    {
        //m_AnimationState = ANIM_STATE_IDLE;
        Audio::m_Soloud.stop(m_FootstepsHandle);
    }
}

void Enemy::PostCollisionUpdate()
{
    double dt = GetDeltaTime();
    m_pStateMachine->Update();
    SetAnimState(&m_Model, m_AnimationState);
    UpdateModel(&m_Model, (float)dt);

    /*
    if ( GetDebugSettings()->patrol )
    {
        if ( m_pPath != nullptr && !m_Target.empty() )
        {
            this->Patrol();
        }
    }
    else if ( GetDebugSettings()->wander )
    {
        this->Wander();
    }
    else
    {
        this->Idle();
    }
    */
}

void Enemy::LoadModel(const char* path, glm::vec3 initialPosition)
{
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model             = CreateModelFromIQM(&iqmModel);
    m_Model.isRigidBody = false;
    m_Model.renderFlags = MODEL_RENDER_FLAG_NONE;
    m_Model.scale       = glm::vec3(1.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ )
    {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;
        ec->center      = initialPosition + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace    = glm::scale(glm::mat4(1.0f), scale);
    }

    m_Model.position.z -= GetEllipsoidColliderPtr()->radiusB;

    glm::quat modelForwardFix = glm::angleAxis(glm::radians(180.0f), DOD_WORLD_UP);
    m_Model.orientation       = modelForwardFix;

    SetAnimState(&m_Model, ANIM_STATE_WALK);
}

EllipsoidCollider* Enemy::GetEllipsoidColliderPtr()
{
    return &m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
}

HKD_Model* Enemy::GetModel()
{
    return &m_Model;
}

void Enemy::UpdatePosition(glm::vec3 newPosition)
{
    m_Position = newPosition;
}

bool Enemy::HandleMessage(const Telegram& telegram)
{
    return m_pStateMachine->HandleMessage(telegram);
}
