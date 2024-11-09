//
// Created by benek on 10/14/24.
//

#ifndef BASEGAMEENTITY_H
#define BASEGAMEENTITY_H
#include "../Message/telegram.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

// NOTE: (Michael): This is essentially what a discriminated union would give us in C.
// TODO: (Michael): Game devs must be able to define their own ET_*. That would not go here
// into this file! Think about this! (Look at Quake again!).
enum EntityType {
    ET_PLAYER,
    ET_ENEMY,
    ET_DOOR,
    ET_CAMERA,
    ET_FLY_CAMERA
};

class BaseGameEntity {
private:
    int m_ID{};

    // this must be called within the constructor to make sure the ID is set
    // correctly. It verifies that the value passed to the method is greater
    // or equal to the next valid ID, before setting the ID and incrementing
    // the next valid ID
    void SetID(int value);
    
    EntityType m_Type;

public:
    // this is the next valid ID. Each time a BaseGameEntity is instantiated
    // this value is updated
    static int m_iNextValidID;

    explicit BaseGameEntity(const int id, EntityType type) {
        SetID(id);
        m_Type = type;
    }

    virtual ~BaseGameEntity() = default;

    // all entities must implement an update function
    virtual void Update() = 0;

    // all entities can communicate using messages. They are sent
    // using the MessageDispatcher singleton class
    virtual bool HandleMessage(const Telegram& telegram) = 0;

    EntityType Type() const {
        return m_Type;
    }

    [[nodiscard]] int ID() const {
        return m_ID;
    }
    
    glm::vec3  m_Position = glm::vec3(0.0f);
};

#endif // BASEGAMEENTITY_H

