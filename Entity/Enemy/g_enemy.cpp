//
// Created by benek on 10/14/24.
//

#include "g_enemy.h"
#include "../../input.h"
#include "../../utils.h"
#include "g_enemy_states.h"
#include <SDL.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/gtx/vector_angle.hpp"

Enemy::Enemy(const int id)
    : MovingEntity(id, ET_ENEMY),
      m_pStateMachine(nullptr),
      m_AnimationState(ANIM_STATE_IDLE),
      m_EllipsoidCollider() {
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(EnemyIdle::Instance());

    LoadModel("models/multiple_anims/multiple_anims.iqm", glm::vec3(0.0f, 0.0f, 50.0f));
    m_Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    m_pSteeringBehaviour = new SteeringBehaviour(this);
    m_pSteeringBehaviour->WanderOn();
}

void Enemy::Update() {
    double dt = GetDeltaTime();
    m_pStateMachine->Update();
    //printf("Enemy Health: %f\n", m_Health);

    glm::vec3 force = m_pSteeringBehaviour->Calculate();
    glm::vec3 acceleration = force / (float)m_Mass;
    //update velocity
    m_Velocity += acceleration * (float)GetDeltaTime() / 1000.0f;
    m_Velocity = Truncate(m_Velocity, m_MaxSpeed);

    if ( Speed() > 0.00000001 ) {

        // glm::
        glm::quat rot = glm::angleAxis(glm::radians(glm::angle(m_Forward, m_Velocity)), m_Up);
        m_Model.orientation *= rot;
        m_Forward = glm::normalize(m_Velocity);
        m_Side = glm::cross(m_Forward, m_Up);
    }

    // if ( KeyPressed(SDLK_o) ) {
    //     m_Velocity.x += 0.01;
    //     m_AnimationState = ANIM_STATE_WALK;
    // } else if ( KeyPressed(SDLK_l) ) {
    //     m_Velocity.x -= 0.01;
    // } else {
    //     m_Velocity.x = 0.0f;
    // }

    if ( Speed() >= 0.00001f ) {
        m_AnimationState = ANIM_STATE_WALK;
    } else if ( Speed() > 0.02f ) {
        m_AnimationState = ANIM_STATE_RUN;
    } else {
        m_AnimationState = ANIM_STATE_IDLE;
    }
    SetAnimState(&m_Model, m_AnimationState);
    UpdateModel(&m_Model, (float)dt);
}

void Enemy::LoadModel(const char* path, glm::vec3 initialPosition) {
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

EllipsoidCollider Enemy::GetEllipsoidCollider() const {
    return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
}

HKD_Model* Enemy::GetModel() {
    return &m_Model;
}

void Enemy::UpdatePosition(glm::vec3 newPosition) {
    // Update the ellipsoid colliders for all animation states based on the new collision position
    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        m_Model.ellipsoidColliders[ i ].center = newPosition;
    }
    m_Model.position.x = newPosition.x;
    m_Model.position.y = newPosition.y;
    m_Model.position.z = newPosition.z - GetEllipsoidCollider().radiusB;
    m_Position = newPosition;
}

bool Enemy::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}
