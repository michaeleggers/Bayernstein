//
// Created by me on 10/14/24.
//

#include "g_fp_player.h"

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

#include "../../Audio/Audio.h"
#include "../../Message/message_dispatcher.h"
#include "../../Message/message_type.h"
#include "../../globals.h"
#include "../../hkd_interface.h"
#include "../../input.h"
#include "../../input_handler.h"
#include "../../input_receiver.h"
#include "../../utils/utils.h"
#include "../Weapon/g_weapon.h"
#include "../entity_manager.h"
#include "g_fp_player_states.h"
#include "imgui.h"

FirstPersonPlayer::FirstPersonPlayer(const std::vector<Property>& properties)
    : MovingEntity(ET_PLAYER),
      m_pStateMachine(nullptr),
      m_AnimState(ANIM_STATE_IDLE)
{
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(FirstPersonPlayerIdle::Instance());
    BaseGameEntity::GetProperty<glm::vec3>(properties, "origin", &m_Position);
    LoadModel("models/multiple_anims/multiple_anims.iqm", m_Position);
    m_PrevPosition = m_Position;
    //m_PrevPosition.z += GetEllipsoidColliderPtr()->radiusB;
    m_Camera = Camera(m_Position);
    //m_Camera.RotateAroundSide(-60.0f);
    m_PrevCollisionState = ES_UNDEFINED;
    m_Momentum           = glm::vec3(0.0f);

    m_SfxJump    = Audio::LoadSource("sfx/jump_01.wav", 0.5f);
    m_SfxFootsteps
        = Audio::LoadSource("sfx/sonniss/015_Foley_Footsteps_Asphalt_Boot_Walk_Fast_Run_Jog_Close.wav", 1.0f, true);

    m_Weapon              = new Weapon(properties);
    IRender* renderer     = GetRenderer();
    int      hWeaponModel = renderer->RegisterModel(m_Weapon->GetModel());
}

// FIX: At the moment called by the game itself.
// Must be called before entity system updates entities.
// (Calls ::Update() on Entities).
void FirstPersonPlayer::UpdatePosition(glm::vec3 newPosition)
{
    m_Position = newPosition;
}

void FirstPersonPlayer::PreCollisionUpdate()
{

    double        dt          = GetDeltaTime();
    static double accumulator = 0.0;
    accumulator += dt;

    if ( m_PrevCollisionState == ES_ON_GROUND )
    {
        if ( m_WantsToJump )
        {
            printf("Jumping....\n");
            Audio::m_SfxBus.play(*m_SfxJump, -1);
            m_FlyMomentum   = m_Momentum;
            m_FlyMomentum.z = JUMPING_MOMENTUM;
        }
    }

    while ( accumulator >= DOD_FIXED_UPDATE_TIME )
    {

        if ( m_PrevCollisionState == ES_ON_GROUND )
        {

            glm::vec3 horizontalMomentum = glm::vec3(m_Momentum.x, m_Momentum.y, 0.0f);

            if ( m_IsMoving )
            {
                m_Momentum += (float)DOD_FIXED_UPDATE_TIME * m_Dir * GROUND_RESISTANCE;
                if ( glm::length(horizontalMomentum) > m_MovementSpeed )
                {
                    m_Momentum = m_MovementSpeed * glm::normalize(m_Momentum);
                }
            }
            else
            {
                if ( glm::length(horizontalMomentum) > 0.0f )
                {
                    glm::vec3 frictionForce
                        = (float)DOD_FIXED_UPDATE_TIME * -glm::normalize(horizontalMomentum) * GROUND_FRICTION;
                    if ( glm::length(frictionForce) > glm::length(horizontalMomentum) )
                    {
                        m_Momentum.x = 0.0f;
                        m_Momentum.y = 0.0f;
                    }
                    else
                    {
                        m_Momentum.x += frictionForce.x;
                        m_Momentum.y += frictionForce.y;
                    }
                }
            }
        }
        else if ( m_PrevCollisionState == ES_IN_AIR )
        {
            glm::vec3 horizontalMomentum = glm::vec3(m_Momentum.x, m_Momentum.y, 0.0f);
            m_Momentum += (float)DOD_FIXED_UPDATE_TIME * (1.0f - IN_AIR_FRICTION) * m_Dir * GROUND_RESISTANCE;
            if ( glm::length(horizontalMomentum) > m_MovementSpeed )
            {
                m_Momentum = m_MovementSpeed * glm::normalize(m_Momentum);
            }
            // If in air, apply some downward gravity acceleration.
            m_FlyMomentum.z += (float)DOD_FIXED_UPDATE_TIME * (-GRAVITY_ACCELERATION);
        }

        // NOTE: It *can* make sense to restrict further
        // downward gravity in certain cases, eg. when
        // the gravity is set to something pretty extrem
        // and it would cause the player to fall so quickly
        // that after a few frames the numeric precision gets
        // exhausted (yes, this can happen ;) ). Usually  this
        // is not needed, though.
        /*
        if (glm::length(m_FlyMomentum) > JUMPING_MOMENTUM) {
            m_FlyMomentum = JUMPING_MOMENTUM * glm::normalize(m_FlyMomentum);
        } 
        */

        m_Momentum.z = glm::clamp(m_Momentum.z, 0.0f, JUMPING_MOMENTUM);

        accumulator -= DOD_FIXED_UPDATE_TIME;
    } // while accumulator > DOD_FIXED_UPDATE_TIME
    m_Velocity = m_Momentum + m_FlyMomentum;
}

void FirstPersonPlayer::PostCollisionUpdate()
{

    double dt = GetDeltaTime();

    ImGui::Begin("FPS Player Info");
    if ( m_CollisionState == ES_IN_AIR )
    {
        ImGui::Text("flying!\n");
    }
    else
    {
        ImGui::Text("on ground!\n");
    }
    ImGui::Text("m_Velocity.z: %f", m_Velocity.z);
    ImGui::Text("m_Momentum.xyz: %f, %f, %f", m_Momentum.x, m_Momentum.y, m_Momentum.z);
    ImGui::End();

    if ( m_PrevCollisionState == ES_ON_GROUND && m_CollisionState == ES_IN_AIR )
    {
        printf("switched from on ground to in air.\n");
        // If the footsteps are playing, turn them off now!
        // TODO: Is it ok to not check if the handle is even valid before calling stop()?
        Audio::m_Soloud.stop(m_FootstepsHandle);
    }
    else if ( m_PrevCollisionState == ES_IN_AIR && m_CollisionState == ES_ON_GROUND )
    {
        printf("switched from in air to on ground.\n");

        m_FlyMomentum = glm::vec3(0.0f);
    }

    m_PrevCollisionState = m_CollisionState;

    if ( KeyPressed(SDLK_w) )
    {
        m_pStateMachine->ChangeState(FirstPersonPlayerRunning::Instance());
    }
    else
    {
        m_pStateMachine->ChangeState(FirstPersonPlayerIdle::Instance());
    }

    UpdateModel(&m_Model, (float)dt);

    //m_Position = m_Model.position;
    m_Camera.m_Pos = m_Position;
    // Adjust the camera so it is roughly at the top of the model's head.
    m_Camera.m_Pos += glm::vec3(0.0f, 0.0f, GetEllipsoidColliderPtr()->radiusB);
    //m_Camera.Pan( -100.0f * m_Camera.m_Forward );

    // Adjust the Weapon model
    glm::vec3 weaponPos = m_Camera.m_Pos;
    //weaponPos           = glm::rotate(m_Camera.m_Orientation, weaponPos);
    //weaponPos.z -= 5.0f;
    //weaponPos.x += 5.0f;
    weaponPos += 20.0f * m_Camera.m_Forward;
    weaponPos += 4.0f * m_Camera.m_Side;
    weaponPos += -2.0f * m_Camera.m_Up;
    //m_Weapon->UpdatePosition(glm::vec3(viewSpaceWeaponPos.x, viewSpaceWeaponPos.y, viewSpaceWeaponPos.z));
    m_Weapon->UpdatePosition(weaponPos);
    m_Weapon->m_Orientation = m_Camera.m_Orientation;

    m_pStateMachine->Update();
}

void FirstPersonPlayer::LoadModel(const char* path, glm::vec3 initialPosition)
{
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model        = CreateModelFromIQM(&iqmModel);
    m_Model.pOwner = this;
    m_Model.renderFlags |= MODEL_RENDER_FLAG_IGNORE;
    m_Model.isRigidBody = false;
    m_Model.scale       = glm::vec3(30.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ )
    {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        //ec->radiusA           = 2.0f;
        //ec->radiusB           = 2.0f;
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;

        // Add the vertical radius of the collider so we make sure we start
        // *over* the floor.
        ec->center      = initialPosition + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace    = glm::scale(glm::mat4(1.0f), scale);
    }

    // Model is defined with origin at its feet. Move it down to be at the ground.
    m_Model.position.z -= GetEllipsoidColliderPtr()->radiusB;

    // The model was modeled with -y = forward, but we use +y = forward.
    // So, we rotate the model initially by 180 degrees.
    glm::quat modelForwardFix = glm::angleAxis(glm::radians(180.0f), DOD_WORLD_UP);
    m_Model.orientation       = modelForwardFix;

    SetAnimState(&m_Model, ANIM_STATE_WALK);
}

// NOTE: This is not being used now as the entity should not own
// and/or update a camera by itself. The camera is an entity itself.
// We leave it here anyways, because this shows how it could be done
// if it turns out that camera-entities are dumb (I don't think so
// but still...).
void FirstPersonPlayer::UpdateCamera(Camera* camera)
{
    // Fix camera position
    //camera->m_Pos.x = m_Model.position.x;
    //camera->m_Pos.y = m_Model.position.y;
    //camera->m_Pos.z = m_Model.position.z + 70.0f;
    //camera->m_Pos += (-m_Forward * 80.0f);
    //// m_RotationAngle should already have the information about if we want to move left or right
    //if (KeyPressed(SDLK_RIGHT) || KeyPressed(SDLK_LEFT)) {
    //    camera->RotateAroundUp(m_RotationAngle);
    //}
}

void FirstPersonPlayer::UpdatePlayerModel()
{

    ButtonState forward   = CHECK_ACTION("forward");
    ButtonState back      = CHECK_ACTION("back");
    ButtonState left      = CHECK_ACTION("left");
    ButtonState right     = CHECK_ACTION("right");
    ButtonState speed     = CHECK_ACTION("speed");
    ButtonState fire      = CHECK_ACTION("fire");
    ButtonState jump      = CHECK_ACTION("jump");
    ButtonState turnLeft  = CHECK_ACTION("turn_left");
    ButtonState turnRight = CHECK_ACTION("turn_right");
    ButtonState mouseLook = CHECK_ACTION("mlook");

    double dt = GetDeltaTime();

    glm::quat qYaw = glm::angleAxis(glm::radians(-m_Yaw), DOD_WORLD_UP);
    if ( mouseLook == ButtonState::MOVED )
    {
        const MouseMotion mouseMotion = GetMouseMotion();
        m_MouseX                      = mouseMotion.current.xrel;
        m_MouseY                      = mouseMotion.current.yrel;

        // Compute rotation angles based on mouse input
        float rotAngleUp   = dt / 1000.0f * m_LookSpeed * (float)m_MouseX;
        float rotAngleSide = dt / 1000.0f * m_LookSpeed * (float)m_MouseY;

        m_Pitch += rotAngleSide;
        m_Yaw += rotAngleUp;

        m_Pitch          = glm::clamp(m_Pitch, -MAX_MOUSE_LOOK_DEGREES, MAX_MOUSE_LOOK_DEGREES);
        glm::quat qPitch = glm::angleAxis(glm::radians(-m_Pitch), DOD_WORLD_RIGHT);
        qYaw             = glm::angleAxis(glm::radians(-m_Yaw), DOD_WORLD_UP);
        glm::quat qTotal = qYaw * qPitch;

        m_Camera.m_Forward     = glm::rotate(qTotal, DOD_WORLD_FORWARD);
        m_Camera.m_Up          = glm::rotate(qTotal, DOD_WORLD_UP);
        m_Camera.m_Side        = glm::rotate(qTotal, DOD_WORLD_RIGHT);
        m_Camera.m_Orientation = qTotal;

        // Don't forget the entity's own rotation.
        m_Orientation = qYaw;
    }

    m_Forward = glm::rotate(m_Orientation, DOD_WORLD_FORWARD);
    m_Side    = glm::cross(m_Forward, m_Up);

    m_MovementSpeed = RUN_VELOCITY;
    if ( KeyPressed(SDLK_LSHIFT) )
    {
        m_MovementSpeed *= WALK_FACTOR;
    }
    m_AnimState = ANIM_STATE_IDLE;
    m_Dir       = glm::vec3(0.0f);
    if ( forward == ButtonState::PRESSED )
    {
        m_Dir += m_Forward;
        m_AnimState = ANIM_STATE_RUN;
    }
    if ( back == ButtonState::PRESSED )
    {
        m_Dir -= m_Forward;
        m_AnimState = ANIM_STATE_RUN;
    }
    if ( right == ButtonState::PRESSED )
    {
        m_Dir += m_Side;
        m_AnimState = ANIM_STATE_RUN;
    }
    if ( left == ButtonState::PRESSED )
    {
        m_Dir -= m_Side;
        m_AnimState = ANIM_STATE_RUN;
    }

    // Make sure we don't get faster when going both
    // forward and to the side for example.
    float inputLength = glm::length(m_Dir);
    if ( inputLength > 0.0f )
    { // This is neccessary to not divide by 0 in glm::normalize
        m_Dir = glm::normalize(m_Dir);
    }

    m_IsMoving    = inputLength > 0.0f;
    m_WantsToJump = jump == ButtonState::WENT_DOWN;

    if ( m_AnimState == ANIM_STATE_RUN )
    {

        if ( speed == ButtonState::PRESSED )
        {
            m_AnimState = ANIM_STATE_WALK;
        }

        if ( m_PrevCollisionState == ES_ON_GROUND )
        {
            // Get the handle if not already created.
            if ( !Audio::m_Soloud.isValidVoiceHandle(m_FootstepsHandle) )
            {
                m_FootstepsHandle = Audio::m_SfxBus.play(*m_SfxFootsteps);
            }

            if ( m_AnimState == ANIM_STATE_RUN )
            {
                Audio::m_Soloud.setRelativePlaySpeed(m_FootstepsHandle, 0.9f);
                Audio::m_Soloud.setVolume(m_FootstepsHandle, m_SfxFootsteps->mVolume);
            }
            else if ( m_AnimState == ANIM_STATE_WALK )
            {
                Audio::m_Soloud.setRelativePlaySpeed(m_FootstepsHandle, 0.6f);
                Audio::m_Soloud.setVolume(m_FootstepsHandle, m_SfxFootsteps->mVolume * 0.4f);
            }
        }
    }
    else
    {
        // TODO: Is it ok to not check if the handle is even valid before calling stop()?
        Audio::m_Soloud.stop(m_FootstepsHandle);
    }


    bool fireRequested = fire == ButtonState::WENT_DOWN || fire == ButtonState::PRESSED;
    if ( fireRequested && m_Weapon->Fire() )
    {
        // Trace ray against enemies

        EntityManager*               m_pEntityManager = EntityManager::Instance();
        std::vector<BaseGameEntity*> pAllEntities     = m_pEntityManager->Entities();
        for ( int i = 0; i < pAllEntities.size(); i++ )
        {
            BaseGameEntity* pEntity = pAllEntities[ i ];
            if ( pEntity->Type() == ET_ENEMY )
            {
                Enemy*             pEnemy   = (Enemy*)pEntity;
                EllipsoidCollider* pEC      = pEnemy->GetEllipsoidColliderPtr();
                glm::vec3          enemyPos = pEnemy->m_Position;
                EllipsoidCollider  ec       = *pEC;
                ec.center += enemyPos;
                if ( TraceRayAgainstEllipsoid(m_Camera.m_Pos, m_Camera.m_Forward, *pEC) )
                {
                    Dispatcher->DispatchMessage(SEND_MSG_IMMEDIATELY, ID(), pEnemy->ID(), message_type::RayHit, 0);
                }
            }
        }
    }

    SetAnimState(&m_Model, m_AnimState);

    // TODO: This *must* only be in *one* entity that gets updated. Otherwise
    // the sound 'jumps' from position to positon. We could actually
    // move this code out of the entity class and attach it to a specific
    // entity at a higher level in code.
    Audio::m_Soloud.set3dListenerPosition(m_Position.x, m_Position.y, m_Position.z);
    Audio::m_Soloud.set3dListenerAt(m_Forward.x, m_Forward.y, m_Forward.z);
    Audio::m_Soloud.set3dListenerVelocity(m_Velocity.x, m_Velocity.y, m_Velocity.z);
    Audio::m_Soloud.set3dListenerUp(DOD_WORLD_UP.x, DOD_WORLD_UP.y, DOD_WORLD_UP.z);
    Audio::m_Soloud.update3dAudio();
}

EllipsoidCollider* FirstPersonPlayer::GetEllipsoidColliderPtr()
{
    //return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
    return &m_Model.ellipsoidColliders[ 0 ];
}

HKD_Model* FirstPersonPlayer::GetModel()
{
    return &m_Model;
}

bool FirstPersonPlayer::HandleMessage(const Telegram& telegram)
{
    return m_pStateMachine->HandleMessage(telegram);
}

void FirstPersonPlayer::HandleInput()
{
    ButtonState captainState = CHECK_ACTION("set_captain");
    if ( captainState == ButtonState::WENT_DOWN )
    {
        printf("FirstPersonPlayer: I am the captain!\n");
    }
    UpdatePlayerModel();
}
