//
// Created by benek on 10/14/24.
//

#include "g_player.h"

#include "../../input.h"
#include "../../utils.h"
#include "g_player_states.h"
#include <SDL.h>

Player::Player(const int id, glm::vec3 initialPosition)
    : MovingEntity(id, ET_PLAYER),
      m_pStateMachine(nullptr),
      m_AnimationState(ANIM_STATE_IDLE) {
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(PlayerIdle::Instance());
    LoadModel("models/multiple_anims/multiple_anims.iqm", initialPosition);
}

void Player::Update() {

    if ( KeyPressed(SDLK_w) ) {
        m_pStateMachine->ChangeState(PlayerRunning::Instance());
    } else if ( KeyPressed(SDLK_SPACE) ) {
        m_pStateMachine->ChangeState(PlayerAttacking::Instance());
    } else {
        m_pStateMachine->ChangeState(PlayerIdle::Instance());
    }

    UpdatePlayerModel();
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

void Player::UpdateCamera(Camera* camera) {
    // Fix camera position
    camera->m_Pos.x = m_Model.position.x;
    camera->m_Pos.y = m_Model.position.y;
    camera->m_Pos.z = m_Model.position.z + 70.0f;
    camera->m_Pos += (-m_Forward * 80.0f);
    // m_RotationAngle should already have the information about if we want to move left or right
    if ( KeyPressed(SDLK_RIGHT) || KeyPressed(SDLK_LEFT) ) {
        camera->RotateAroundUp(m_RotationAngle);
    }
}

void Player::UpdatePlayerModel() {
    double dt = GetDeltaTime();
    float followCamSpeed = 0.03f;
    float followTurnSpeed = 0.1f;
    if ( KeyPressed(SDLK_LSHIFT) ) {
        followCamSpeed *= 0.3f;
        followTurnSpeed *= 0.3f;
    }

    // Model rotation
    m_RotationAngle = followTurnSpeed * (float)dt;
    if ( KeyPressed(SDLK_RIGHT) ) {
        m_RotationAngle = -m_RotationAngle;
        glm::quat rot = glm::angleAxis(glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        m_Model.orientation *= rot;
    }
    if ( KeyPressed(SDLK_LEFT) ) {
        glm::quat rot = glm::angleAxis(glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        m_Model.orientation *= rot;
    }

    m_Forward = glm::rotate(m_Model.orientation,
                            glm::vec3(0.0f, -1.0f, 0.0f)); // -1 because the model is facing -1 (Outside the screen)
    m_Side = glm::cross(m_Forward, glm::vec3(0.0f, 0.0f, 1.0f));

    // Change player's velocity and animation state based on input
    m_Velocity = glm::vec3(0.0f);
    float t = (float)dt * followCamSpeed;
    AnimState playerAnimState = ANIM_STATE_IDLE;
    if ( KeyPressed(SDLK_w) ) {
        m_Velocity += t * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( KeyPressed(SDLK_s) ) {
        m_Velocity -= t * m_Forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( KeyPressed(SDLK_d) ) {
        m_Velocity += t * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }
    if ( KeyPressed(SDLK_a) ) {
        m_Velocity -= t * m_Side;
        playerAnimState = ANIM_STATE_RUN;
    }

    if ( playerAnimState == ANIM_STATE_RUN ) {
        if ( KeyPressed(SDLK_LSHIFT) ) {
            playerAnimState = ANIM_STATE_WALK;
        }
    }

    SetAnimState(&m_Model, playerAnimState);

    UpdateModel(&m_Model, (float)dt);
}

void Player::UpdatePosition(glm::vec3 newPosition) {

    // Update the ellipsoid colliders for all animation states based on the new collision position
    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        m_Model.ellipsoidColliders[ i ].center = newPosition;
    }
    m_Model.position.x = newPosition.x;
    m_Model.position.y = newPosition.y;
    m_Model.position.z = newPosition.z - GetEllipsoidCollider().radiusB;
}

EllipsoidCollider Player::GetEllipsoidCollider() const {
    return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
}

HKD_Model* Player::GetModel() {
    return &m_Model;
}

glm::vec3 Player::GetVelocity() const {
    return m_Velocity;
}

bool Player::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}
