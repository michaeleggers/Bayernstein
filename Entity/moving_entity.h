//
// Created by benek on 10/26/24.
//

#ifndef MOVINGENTITY_H
#define MOVINGENTITY_H
#include "./base_game_entity.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

class MovingEntity : public BaseGameEntity {

  public:
    MovingEntity(const int id, EntityType type)
        : BaseGameEntity(id, type),
          m_Velocity(0.0f),
          m_Forward(0.0f, -1.0f, 0.0f),
          m_Side(1.0f, 0.0f, 0.0f),
          m_Up(0.0f, 0.0f, 1.0f),
          m_Mass(1.0f),
          m_MaxSpeed(0.1f),
          m_MaxForce(50.5f),
          m_MaxTurnRate(1.0f) {};

    virtual ~MovingEntity() = default;

    bool IsSpeedMaxedOut() const {
        return glm::pow(m_MaxSpeed, 2) >= glm::pow(glm::length(m_Velocity), 2);
    }
    float Speed() const {
        return glm::length(m_Velocity);
    }
    float SpeedSq() const {
        return glm::pow(glm::length(m_Velocity), 2);
    }

    // all entities must implement an update function
    virtual void Update() = 0;

    // all entities can communicate using messages. They are sent
    // using the MessageDispatcher singleton class
    virtual bool HandleMessage(const Telegram& telegram) = 0;
    
    glm::vec3 m_Velocity;

    //a normalized vector pointing in the direction the entity is heading.
    glm::vec3 m_Forward;

    //a vector perpendicular to the heading vector
    glm::vec3 m_Side;
    glm::vec3 m_Up;

    float m_Mass;

    //the maximum speed this entity may travel at.
    float m_MaxSpeed;

    //the maximum force this entity can produce to power itself
    //(think rockets and thrust)
    float m_MaxForce;

    //the maximum rate (radians per second)this vehicle can rotate
    float m_MaxTurnRate;
};

#endif // MOVINGENTITY_H

