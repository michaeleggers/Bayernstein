//
// Created by benek on 10/14/24.
//

#include "g_player.h"

#include "../../input.h"
#include "../../utils/utils.h"
#include "../../input_handler.h"
#include "../../input_receiver.h"
#include "g_player_states.h"

#include <SDL.h>

Player::Player(glm::vec3 initialPosition)
    : MovingEntity(ET_PLAYER),
      m_pStateMachine(nullptr),
      m_AnimationState(ANIM_STATE_IDLE) {
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(PlayerIdle::Instance());
    LoadModel("models/multiple_anims/multiple_anims.iqm", initialPosition);
    m_Position = initialPosition;
}

// FIX: At the moment called by the game itself.
// Must be called before entity system updates entities.
// (Calls ::Update() on Entities).
void Player::UpdatePosition(glm::vec3 newPosition) {

    // Update the ellipsoid colliders for all animation states based on the new collision position
    for (int i = 0; i < m_Model.animations.size(); i++) {
        m_Model.ellipsoidColliders[i].center = newPosition;
    }
    m_Model.position.x = newPosition.x;
    m_Model.position.y = newPosition.y;
    m_Model.position.z = newPosition.z - GetEllipsoidCollider().radiusB;
}

void Player::Update() {

    if ( KeyPressed(SDLK_w) ) {
        m_pStateMachine->ChangeState(PlayerRunning::Instance());
    } else if ( KeyPressed(SDLK_SPACE) ) {
        m_pStateMachine->ChangeState(PlayerAttacking::Instance());
    } else {
        m_pStateMachine->ChangeState(PlayerIdle::Instance());
    }

    //UpdatePlayerModel();

    double dt = GetDeltaTime();
    UpdateModel(&m_Model, (float)dt);

    m_Position = m_Model.position;

    m_pStateMachine->Update();
}

void Player::LoadModel(const char* path, glm::vec3 initialPosition) {
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model = CreateModelFromIQM(&iqmModel);
    m_Model.isRigidBody = false;
    m_Model.position = initialPosition;
    m_Model.scale = glm::vec3(22.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;
        ec->center = m_Model.position + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
    }

    SetAnimState(&m_Model, ANIM_STATE_WALK);
    m_EllipsoidCollider = GetEllipsoidCollider();
}

// NOTE: This is not being used now as the entity should not own
// and/or update a camera by itself. The camera is an entity itself.
// We leave it here anyways, because this shows how it could be done
// if it turns out that camera-entities are dumb (I don't think so
// but still...).
void Player::UpdateCamera(Camera* camera) {
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

void Player::UpdatePlayerModel() {
   
    ButtonState forward = CHECK_ACTION("forward");
    ButtonState back = CHECK_ACTION("back");
    ButtonState left = CHECK_ACTION("left");
    ButtonState right = CHECK_ACTION("right");
    ButtonState speed = CHECK_ACTION("speed");
    ButtonState turnLeft = CHECK_ACTION("turn_left");
    ButtonState turnRight = CHECK_ACTION("turn_right");
    
    double dt = GetDeltaTime();
    float followCamSpeed = 0.03f;
    float followTurnSpeed = 0.3f;
    if ( KeyPressed(SDLK_LSHIFT) ) {
        followCamSpeed *= 0.3f;
        followTurnSpeed *= 0.3f;
    }

    // Model rotation
    if ( turnLeft == ButtonState::PRESSED ) {
        m_RotationAngle += followTurnSpeed * (float)dt; // TODO: Should be a quaternion in base game entity.
    }
    if ( turnRight == ButtonState::PRESSED ) {
        m_RotationAngle -= followTurnSpeed * (float)dt;
    }
   
    // TODO: If dealing with m_Orientation quaternion, this test can be omitted.
    if (m_RotationAngle >= 360.0f) {
        m_RotationAngle = 0.0f;
    }
    else if (m_RotationAngle < 0.0f) {
        m_RotationAngle = 360.0f;
    }

    glm::quat rot = glm::angleAxis(glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    m_Model.orientation = rot;

    m_Forward = glm::rotate(m_Model.orientation,
                            glm::vec3(0.0f, -1.0f, 0.0f)); // -1 because the model is facing -1 (Outside the screen)
    m_Side = glm::cross(m_Forward, m_Up);

    // Change player's velocity and animation state based on input
    m_Velocity = glm::vec3(0.0f);
    float t = (float)dt * followCamSpeed;
    AnimState playerAnimState = ANIM_STATE_IDLE;
    
    if ( forward == ButtonState::PRESSED ) {
        m_Velocity += t * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( back == ButtonState::PRESSED ) {
        m_Velocity -= t * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( right == ButtonState::PRESSED ) {
        m_Velocity += t * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( left == ButtonState::PRESSED ) {
        m_Velocity -= t * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }

    if (playerAnimState == ANIM_STATE_RUN) {
        if ( speed == ButtonState::PRESSED ) {
            playerAnimState = ANIM_STATE_WALK;
        }
    }

    // Test the input handler here.
    ButtonState jumpState = CHECK_ACTION("jump");
    if (jumpState == ButtonState::PRESSED) {
        printf("I am jumping!\n");
    }
    ButtonState fireState = CHECK_ACTION("fire");
    if (fireState == ButtonState::PRESSED) {
        printf("FIRE!\n");
    }
    ButtonState prevWeapon = CHECK_ACTION("switch_to_prev_weapon");
    if (prevWeapon == ButtonState::WENT_DOWN) {
        printf("Switching to previous weapon!\n");
    }
    
    SetAnimState(&m_Model, playerAnimState);

    // UpdateModel(&m_Model, (float)dt);
}

EllipsoidCollider Player::GetEllipsoidCollider() const {
    return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
}

HKD_Model* Player::GetModel() {
    return &m_Model;
}

bool Player::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}

void Player::HandleInput() {
    ButtonState captainState = CHECK_ACTION("set_captain");
    if ( captainState == ButtonState::WENT_DOWN ) {
        printf("Player: I am the captain!\n");
    }
    UpdatePlayerModel();
}

