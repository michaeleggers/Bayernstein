// benek
// Created by benek on 10/14/24.
//

#include "g_enemy.h"
#include "glm/geometric.hpp"

#include <SDL.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/gtx/vector_angle.hpp"

#include "../../globals.h"
#include "../../input.h"
#include "../../input_handler.h"
#include "../../utils/quick_math.h"
#include "../../utils/utils.h"
#include "g_enemy_states.h"

std::vector<Tri> GenerateVisionCone(
    glm::vec3 position,
    glm::vec3 direction,
    glm::vec3 up,
    float     angle,
    float     range,
    float     height,
    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0)
) {
    std::vector<Tri> tris = {};

    glm::vec3 normalizedDirection = glm::normalize(direction);
    float     halfAngle           = angle / 2.0f;
    float     halfHeight          = height / 2.0f;

    glm::mat3 transformToLeft  = glm::rotate(glm::mat4(1.0f), glm::radians(halfAngle), up);
    glm::mat3 transformToRight = glm::rotate(glm::mat4(1.0f), glm::radians(-halfAngle), up);

    Vertex centerTop    = {.pos = {position.x, position.y, position.z + halfHeight}, .color = color};
    Vertex centerBottom = {.pos = {position.x, position.y, position.z - halfHeight}, .color = color};

    int                 segments  = 8;
    float               angleStep = angle / (float)segments;
    std::vector<Vertex> topVertices;
    std::vector<Vertex> bottomVertices;

    for ( int i = 0; i <= segments; ++i ) {
        float     currentAngle = -halfAngle + (float)i * angleStep;
        glm::mat3 transform    = glm::rotate(glm::mat4(1.0f), glm::radians(currentAngle), up);

        glm::vec3 edge = position + (transform * normalizedDirection * range);

        Vertex topVertex    = {.pos = {edge.x, edge.y, edge.z + halfHeight}, .color = color};
        Vertex bottomVertex = {.pos = {edge.x, edge.y, edge.z - halfHeight}, .color = color};

        topVertices.push_back(topVertex);
        bottomVertices.push_back(bottomVertex);
    }

    for ( int i = 0; i < segments; ++i ) {
        // Top and bottom triangles
        Tri topTri    = {centerTop, topVertices[ i + 1 ], topVertices[ i ]};
        Tri bottomTri = {centerBottom, bottomVertices[ i ], bottomVertices[ i + 1 ]};
        tris.push_back(topTri);
        tris.push_back(bottomTri);

        // Side triangles
        Tri leftSideTop    = {topVertices[ i ], topVertices[ i + 1 ], bottomVertices[ i ]};
        Tri leftSideBottom = {bottomVertices[ i ], topVertices[ i + 1 ], bottomVertices[ i + 1 ]};
        tris.push_back(leftSideTop);
        tris.push_back(leftSideBottom);
    }
    return tris;
};

Enemy::Enemy(const std::vector<Property>& properties)
    : MovingEntity(ET_ENEMY),
      m_AnimationState(ANIM_STATE_IDLE),
      m_EllipsoidCollider() {
    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(EnemyPatrol::Instance());

    // We want position and, if applicable, a target.
    BaseGameEntity::GetProperty<glm::vec3>(properties, "origin", &m_Position);
    // FIX: Mem Leak on exit if target is not being set.
    BaseGameEntity::GetProperty<std::string>(properties, "target", &m_Target);

    LoadModel("models/multiple_anims/multiple_anims.iqm", m_Position);
    m_vision_cone        = GenerateVisionCone(m_Position, m_Forward, m_Up, m_vision_angle, m_range, m_vision_height);
    m_Velocity           = glm::vec3(0.0f, 0.0f, 0.0f);
    m_pSteeringBehaviour = new SteeringBehaviour(this);
    // m_pSteeringBehaviour->WanderOn();
}

void Enemy::Update() {
    double dt = GetDeltaTime();
    m_pStateMachine->Update();

    glm::vec3 force        = m_pSteeringBehaviour->Calculate();
    glm::vec3 acceleration = force / m_Mass;
    //update velocity
    // m_Velocity += acceleration / 1000.0f;
    m_Velocity += acceleration * (float)dt / 1000.0f;
    m_Velocity = math::TruncateVec3(m_Velocity, m_MaxSpeed);

    if ( Speed() > 0.0001 ) {

        // Calculate the new forward direction
        glm::vec3 newForward = glm::normalize(m_Velocity);

        // Apply the rotation to the current orientation
        // NOTE: We set the orientation to -DOD_WORLD_FORWARD because the model is looking to negativ y in
        // model space.
        float     absOrientationAngle   = glm::orientedAngle(-DOD_WORLD_FORWARD, newForward, m_Up);
        glm::quat newForwardOrientation = glm::angleAxis(absOrientationAngle, m_Up);
        m_Model.orientation             = newForwardOrientation;

        // Update the forward and side vectors
        m_Forward = newForward;
        m_Side    = glm::cross(m_Forward, m_Up);
    }

    printf("Speed: %f of %f, %f percent \n", Speed(), m_MaxSpeed, Speed() / m_MaxSpeed);
    if ( Speed() >= 0.0001f ) {
        m_AnimationState = ANIM_STATE_WALK;
        if ( Speed() > m_MaxSpeed * 0.5f ) {
            m_AnimationState = ANIM_STATE_RUN;
        }
    }
    else {
        m_AnimationState = ANIM_STATE_IDLE;
    }

    //------------------------------------
    // vision stuff
    //------------------------------------
    glm::vec3 conePosition = m_Position;
    conePosition.z += GetEllipsoidCollider().radiusB;

    m_PlayerInRange = glm::distance(m_Position, m_pPlayerEnity->m_Position) <= m_range;

    glm::vec3 forwardToPlayer             = glm::normalize(m_pPlayerEnity->m_Position - (m_Position + m_Forward));
    float     angleBetweenEntityAndPlayer = glm::degrees(glm::angle(forwardToPlayer, m_Forward));
    m_PlayerInAngle                       = angleBetweenEntityAndPlayer < (m_vision_angle / 2.0f);

    // this should be considereing the players height as well
    m_PlayerOnSameLevel = m_Position.z + m_vision_height / 2.0f > m_pPlayerEnity->m_Position.z
                          && m_Position.z - m_vision_height / 2.0f < m_pPlayerEnity->m_Position.z;
    printf("in range: %b, in angle: %b, same level %b\n", m_PlayerInRange, m_PlayerInAngle, m_PlayerOnSameLevel);
    m_PlayerDetected = m_PlayerInAngle && m_PlayerInRange;

    if ( m_PlayerDetected ) {
        m_vision_cone
            = GenerateVisionCone(conePosition, m_Forward, DOD_WORLD_UP, 45.0f, 195.0f, 80, glm::vec4(1, 0, 0, 1));
    }
    else {
        m_vision_cone
            = GenerateVisionCone(conePosition, m_Forward, DOD_WORLD_UP, 45.0f, 195.0f, 80, glm::vec4(0, 1, 0, 1));
    }

    //------------------------------------

    SetAnimState(&m_Model, m_AnimationState);
    UpdateModel(&m_Model, (float)dt);
}

void Enemy::LoadModel(const char* path, glm::vec3 initialPosition) {
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model             = CreateModelFromIQM(&iqmModel);
    m_Model.isRigidBody = false;
    m_Model.position    = initialPosition;
    m_Model.scale       = glm::vec3(22.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ ) {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;
        ec->center      = m_Model.position + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace    = glm::scale(glm::mat4(1.0f), scale);
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
    newPosition.z -= GetEllipsoidCollider().radiusB;
    m_Model.position = newPosition;
    m_Position       = newPosition;
    //printf("Position: %f, %f, %f\n", m_Position.x, m_Position.y, m_Position.z);
}

bool Enemy::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}