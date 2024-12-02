// 
// Created by benek on 10/14/24.
//

#include "g_enemy.h"

#include <stdio.h>
#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/gtx/vector_angle.hpp"

#include "../../input.h"
#include "../../utils/quick_math.h"
#include "../../utils/utils.h"
#include "g_enemy_states.h"
#include "../../input_handler.h"

Enemy::Enemy(const std::vector<Property>& properties)
    : MovingEntity(ET_ENEMY),
      m_AnimationState(ANIM_STATE_IDLE) {

    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(EnemyIdle::Instance());
   
    // We want position and, if applicable, a target.
    BaseGameEntity::GetProperty<glm::vec3>(properties, "origin", &m_Position);
    // FIX: Mem Leak on exit if target is not being set.
    BaseGameEntity::GetProperty<std::string>(properties, "target", &m_Target);

    LoadModel("models/multiple_anims/multiple_anims.iqm", m_Position);
    m_Model.pOwner = this;
    m_Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    m_PrevPosition = GetEllipsoidColliderPtr()->center;
    m_Position = m_PrevPosition;
    m_pSteeringBehaviour = new SteeringBehaviour(this);
    //m_pSteeringBehaviour->WanderOn();
}

void Enemy::PreCollisionUpdate() {

    float dt = (float)GetDeltaTime();
    glm::vec3 force = m_pSteeringBehaviour->Calculate();
    glm::vec3 acceleration = force / m_Mass;
    //update velocity
    //m_Velocity += acceleration * 1000.0f;
    m_Velocity += acceleration * dt / 1000.0f;
    m_Velocity = math::TruncateVec3(m_Velocity, m_MaxSpeed);
    if ( Speed() > 0.001 ) {

        // Calculate the new forward direction
        glm::vec3 newForward = glm::normalize(m_Velocity);

        // Calculate the rotation needed to align the current forward direction with the new forward direction
        float rotationAngle = glm::orientedAngle(m_Forward, newForward, m_Up);
        glm::quat rotation = glm::angleAxis(rotationAngle, m_Up);

        // Apply the rotation to the current orientation
        //m_Model.orientation = m_Model.orientation * rotation;
        m_Orientation = m_Orientation * rotation;

        // Update the forward and side vectors
        m_Forward = newForward;
        m_Side = glm::cross(m_Forward, m_Up);
    }

    if ( Speed() >= 0.00001f ) {
        m_AnimationState = ANIM_STATE_WALK;
    } else if ( Speed() > m_MaxSpeed * 0.5f ) {
        m_AnimationState = ANIM_STATE_RUN;
    } else {
        m_AnimationState = ANIM_STATE_IDLE;
    }
}

void Enemy::Update() {
    double dt = GetDeltaTime();
    m_pStateMachine->Update();
    SetAnimState(&m_Model, m_AnimationState);
    UpdateModel(&m_Model, (float)dt);
}

void Enemy::LoadModel(const char* path, glm::vec3 initialPosition) {
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model = CreateModelFromIQM(&iqmModel);
    m_Model.isRigidBody = false;
    m_Model.renderFlags = MODEL_RENDER_FLAG_NONE;
    m_Model.scale = glm::vec3(22.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;
        ec->center = initialPosition + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
    }
  
    m_Model.position.z -= GetEllipsoidColliderPtr()->radiusB;
    
    glm::quat modelForwardFix = glm::angleAxis( glm::radians(180.0f), DOD_WORLD_UP );
    m_Model.orientation = modelForwardFix;

    SetAnimState(&m_Model, ANIM_STATE_WALK);
    m_EllipsoidCollider = GetEllipsoidCollider();
}

EllipsoidCollider* Enemy::GetEllipsoidColliderPtr() {
    return &m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
}

HKD_Model* Enemy::GetModel() {
    return &m_Model;
}

void Enemy::UpdatePosition(glm::vec3 newPosition) {
    m_Position = newPosition;
}

bool Enemy::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}


