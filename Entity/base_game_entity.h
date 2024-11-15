//
// Created by benek on 10/14/24.
//

#ifndef BASEGAMEENTITY_H
#define BASEGAMEENTITY_H

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "../Message/telegram.h"
#include "../collision.h"


// NOTE: (Michael): This is essentially what a discriminated union would give us in C.
// TODO: (Michael): Game devs must be able to define their own ET_*. That would not go here
// into this file! Think about this! (Look at Quake again!).
// FIX: Generate entity type GUID inside user defined game entity class.
enum EntityType {
    ET_PLAYER,
    ET_ENEMY,
    ET_DOOR,
    ET_CAMERA,
    ET_FLY_CAMERA
};

class BaseGameEntity {
  private:
    int m_ID{}; // Set by entity manager.
    EntityType m_Type;

  public:
    explicit BaseGameEntity(EntityType type) {
        m_Type = type;
    }

    virtual ~BaseGameEntity() = default;

    // all entities must implement an update function
    virtual void Update() = 0;
    // FIX: At the moment needed by collision system.
    virtual void UpdatePosition(glm::vec3 newPosition) {};

    // all entities can communicate using messages. They are sent
    // using the MessageDispatcher singleton class
    virtual bool HandleMessage(const Telegram& telegram) = 0;

    EntityType Type() const {
        return m_Type;
    }

    void SetID(int value);
    
    [[nodiscard]] int ID() const {
        return m_ID;
    }
   

    // FIX: Should make pure virtual. Not all entities
    // have this of course, but we want to make things
    // simple for now so we just say every entity has it.
    // Same with position and velocity...
    virtual EllipsoidCollider GetEllipsoidCollider() const {
        return {};
    };
    
    glm::vec3   m_Position = glm::vec3(0.0f);
    glm::vec3   m_Velocity = glm::vec3(0.0f); // TODO: Actually make use of it and remove from subclasses!
    float       m_RotationAngle = 0.0f; // TODO: Should be a quaternion called m_Orientation.
};

#endif // BASEGAMEENTITY_H

