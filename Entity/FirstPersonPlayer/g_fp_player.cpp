//
// Created by me on 10/14/24.
//

#include "g_fp_player.h"

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

#include "../../input.h"
#include "../../utils/utils.h"
#include "../../input_handler.h"
#include "../../input_receiver.h"
#include "../../globals.h"
#include "imgui.h"
#include "g_fp_player_states.h"

FirstPersonPlayer::FirstPersonPlayer(glm::vec3 initialPosition)
    : MovingEntity(ET_PLAYER),
      m_pStateMachine(nullptr),
      m_AnimationState(ANIM_STATE_IDLE) {
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(FirstPersonPlayerIdle::Instance());
    LoadModel("models/multiple_anims/multiple_anims.iqm", initialPosition);
    m_Position = initialPosition;
    m_PrevPosition = initialPosition;
    //m_PrevPosition.z += GetEllipsoidColliderPtr()->radiusB;
    m_Camera = Camera(initialPosition);
    m_PrevCollisionState = ES_UNDEFINED;
}

// FIX: At the moment called by the game itself.
// Must be called before entity system updates entities.
// (Calls ::Update() on Entities).
void FirstPersonPlayer::UpdatePosition(glm::vec3 newPosition) {
    m_Position = newPosition;
}

void FirstPersonPlayer::PostCollisionUpdate() {

    double dt = GetDeltaTime();
    
    ImGui::Begin("FPS Player Info");
    if (m_CollisionState == ES_IN_AIR) {
        ImGui::Text("flying!\n");
    }
    else {
        ImGui::Text("on ground!\n");
    }
    ImGui::Text("m_Velocity.z: %f", m_Velocity.z);
    ImGui::Text("m_Momentum.xyz: %f, %f, %f",
                m_Momentum.x, m_Momentum.y, m_Momentum.z);
    ImGui::End();

    if (m_PrevCollisionState == ES_ON_GROUND && m_CollisionState == ES_IN_AIR) {
        printf("switched from on ground to in air.\n");
    }
    else if (m_PrevCollisionState == ES_IN_AIR && m_CollisionState == ES_ON_GROUND) {
        printf("switched from in air to on ground.\n");
        m_FlyMomentum = glm::vec3(0.0f);
    }
    
    m_PrevCollisionState = m_CollisionState;
    
    
    if ( KeyPressed(SDLK_w) ) {
        m_pStateMachine->ChangeState(FirstPersonPlayerRunning::Instance());
    } else {
        m_pStateMachine->ChangeState(FirstPersonPlayerIdle::Instance());
    }

    UpdateModel(&m_Model, (float)dt);

    //m_Position = m_Model.position;
    m_Camera.m_Pos = m_Position;
    // Adjust the camera so it is roughly at the top of the model's head.
    m_Camera.m_Pos += glm::vec3(0.0f, 0.0f, GetEllipsoidColliderPtr()->radiusB - 24.0f);
    //m_Camera.Pan( -100.0f * m_Camera.m_Forward );

    m_pStateMachine->Update();
}

void FirstPersonPlayer::LoadModel(const char* path, glm::vec3 initialPosition) {
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model = CreateModelFromIQM(&iqmModel);
    m_Model.pOwner = this;
    //m_Model.renderFlags |= MODEL_RENDER_FLAG_IGNORE;
    m_Model.isRigidBody = false;
    m_Model.scale = glm::vec3(30.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;
        
        // Add the vertical radius of the collider so we make sure we start
        // *over* the floor.
        ec->center = initialPosition + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
    }
    
    // Model is defined with origin at its feet. Move it down to be at the ground.
    m_Model.position.z -= GetEllipsoidColliderPtr()->radiusB;

    // The model was modeled with -y = forward, but we use +y = forward. 
    // So, we rotate the model initially by 180 degrees.
    glm::quat modelForwardFix = glm::angleAxis( glm::radians(180.0f), DOD_WORLD_UP );
    m_Model.orientation = modelForwardFix;

    SetAnimState(&m_Model, ANIM_STATE_WALK);
}

// NOTE: This is not being used now as the entity should not own
// and/or update a camera by itself. The camera is an entity itself.
// We leave it here anyways, because this shows how it could be done
// if it turns out that camera-entities are dumb (I don't think so
// but still...).
void FirstPersonPlayer::UpdateCamera(Camera* camera) {
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

void FirstPersonPlayer::UpdatePlayerModel() {
   
    ButtonState forward = CHECK_ACTION("forward");
    ButtonState back = CHECK_ACTION("back");
    ButtonState left = CHECK_ACTION("left");
    ButtonState right = CHECK_ACTION("right");
    ButtonState speed = CHECK_ACTION("speed");
    ButtonState jump = CHECK_ACTION("jump");
    ButtonState turnLeft = CHECK_ACTION("turn_left");
    ButtonState turnRight = CHECK_ACTION("turn_right");
    ButtonState mouseLook = CHECK_ACTION("mlook");
    
    double dt = GetDeltaTime();
    float movementSpeed = RUN_VELOCITY;
    if ( KeyPressed(SDLK_LSHIFT) ) {
        movementSpeed *= WALK_FACTOR;
    }

    glm::quat qYaw = glm::angleAxis( glm::radians(-m_Yaw), DOD_WORLD_UP );
    if ( mouseLook == ButtonState::MOVED ) {
        const MouseMotion mouseMotion = GetMouseMotion();
        m_MouseX = mouseMotion.current.xrel;
        m_MouseY = mouseMotion.current.yrel;
        
        // Compute rotation angles based on mouse input
        float rotAngleUp   = dt/1000.0f * m_LookSpeed * (float)m_MouseX;
        float rotAngleSide = dt/1000.0f * m_LookSpeed * (float)m_MouseY;

        m_Pitch += rotAngleSide;
        m_Yaw   += rotAngleUp;
        
        m_Pitch = glm::clamp( m_Pitch, -MAX_MOUSE_LOOK_DEGREES, MAX_MOUSE_LOOK_DEGREES );
        glm::quat qPitch = glm::angleAxis( glm::radians(-m_Pitch), DOD_WORLD_RIGHT );
        qYaw = glm::angleAxis( glm::radians(-m_Yaw), DOD_WORLD_UP );
        glm::quat qTotal = qYaw * qPitch;

        m_Camera.m_Forward = glm::rotate(qTotal, DOD_WORLD_FORWARD);
        m_Camera.m_Up = glm::rotate(qTotal, DOD_WORLD_UP);
        m_Camera.m_Side = glm::rotate(qTotal, DOD_WORLD_RIGHT);
        m_Camera.m_Orientation = qTotal;

        // Don't forget the entity's own rotation.
        m_Orientation = qYaw;
    }

    m_Forward = glm::rotate(m_Orientation,
                            DOD_WORLD_FORWARD);
    m_Side = glm::cross(m_Forward, m_Up);
    
    // Change player's velocity and animation state based on input
    m_Momentum.x = 0.0f;
    m_Momentum.y = 0.0f;
    m_Momentum.z = 0.0f;
    AnimState playerAnimState = ANIM_STATE_IDLE;
    
    if ( forward == ButtonState::PRESSED ) {
        m_Momentum += movementSpeed * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( back == ButtonState::PRESSED ) {
        m_Momentum -= movementSpeed * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( right == ButtonState::PRESSED ) {
        m_Momentum += movementSpeed * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( left == ButtonState::PRESSED ) {
        m_Momentum -= movementSpeed * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }

    if (playerAnimState == ANIM_STATE_RUN) {
        if ( speed == ButtonState::PRESSED ) {
            playerAnimState = ANIM_STATE_WALK;
        }
    }
    

    if (m_PrevCollisionState == ES_ON_GROUND) {
        if ( jump == ButtonState::WENT_DOWN ) {
            printf("Jumping....\n");
            m_Momentum.z = JUMPING_MOMENTUM;
            m_FlyMomentum = m_Momentum;
        }
    }
    else if (m_PrevCollisionState == ES_IN_AIR) {
        // If in air, apply some downward gravity acceleration.
        m_FlyMomentum.z += (float)dt * (-GRAVITY_ACCELERATION);
        m_Momentum.x *= IN_AIR_FRICTION;
        m_Momentum.y *= IN_AIR_FRICTION;
    }

    m_Momentum +=  m_FlyMomentum;


    m_Velocity = m_Momentum;
    // printf("m_Velocity.z: %f\n", m_Velocity.z);

    ButtonState fireState = CHECK_ACTION("fire");
    if (fireState == ButtonState::PRESSED) {
        printf("FIRE!\n");
    }
    
    SetAnimState(&m_Model, playerAnimState);
}

EllipsoidCollider* FirstPersonPlayer::GetEllipsoidColliderPtr() {
    //return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
    return &m_Model.ellipsoidColliders[ 0 ];
}

HKD_Model* FirstPersonPlayer::GetModel() {
    return &m_Model;
}

bool FirstPersonPlayer::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}

void FirstPersonPlayer::HandleInput() {
    ButtonState captainState = CHECK_ACTION("set_captain");
    if ( captainState == ButtonState::WENT_DOWN ) {
        printf("FirstPersonPlayer: I am the captain!\n");
    }
    UpdatePlayerModel();
}

