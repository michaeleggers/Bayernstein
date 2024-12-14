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
#include "../globals.h"
#include "../map_parser.h"
#include "../utils/utils.h"

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

enum EntityCollisionState {
    ES_UNDEFINED,
    ES_ON_GROUND,
    ES_IN_AIR
};

class BaseGameEntity {
  private:
    int        m_ID{}; // Set by entity manager.
    EntityType m_Type;

  public:
    explicit BaseGameEntity(EntityType type) {
        m_Type           = type;
        m_CollisionState = ES_UNDEFINED;
        m_Orientation    = glm::angleAxis(0.0f, DOD_WORLD_FORWARD);
    }

    template <typename T> bool GetProperty(const std::vector<Property>& properties, std::string propName, T* value) {
        for ( const Property& property : properties ) {
            if ( property.key == propName ) {
                *value = ParseProperty<T>(property.value);
                return true;
            }
        }

        return false;
    }

    template <typename T> T ParseProperty(const std::string& sValue);

    template <> glm::vec3 ParseProperty<glm::vec3>(const std::string& sValue) {
        std::vector<float> origin = ParseValues<float>(sValue);
        return glm::vec3(origin[ 0 ], origin[ 1 ], origin[ 2 ]);
    }

    template <> std::string ParseProperty<std::string>(const std::string& sValue) {
        return sValue;
    }

    template <> float ParseProperty<float>(const std::string& sValue) {
        std::vector<float> floats = ParseValues<float>(sValue);
        return floats[ 0 ];
    }

    template <> double ParseProperty<double>(const std::string& sValue) {
        std::vector<double> floats = ParseValues<double>(sValue);
        return floats[ 0 ];
    }

    template <> int ParseProperty<int>(const std::string& sValue) {
        std::vector<int> ints = ParseValues<int>(sValue);
        return ints[ 0 ];
    }

    virtual ~BaseGameEntity() = default;

    // Call this *before* collision system.
    virtual void PreCollisionUpdate() {};

    // Call this *after* the collision system has run.
    virtual void PostCollisionUpdate() = 0;

    // Call this *after* the collision system has run.
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
    // Ok for now until we use a component for this.
    virtual EllipsoidCollider* GetEllipsoidColliderPtr() {
        return nullptr;
    };

    // Set by the collision system every frame by interpolating
    // between m_PrevPosition and the collisions responses' result.
    glm::vec3 m_Position = glm::vec3(0.0f);

    // Set by the collision system every 'DOD_FIXED_UPDATE_TIME'.
    glm::vec3 m_PrevPosition = glm::vec3(0.0f);

    glm::vec3            m_Velocity = glm::vec3(0.0f);
    glm::quat            m_Orientation;
    float                m_RotationAngle = 0.0f; // TODO: Update follow cam and player to not use this.
    std::string          m_Target        = "";
    EntityCollisionState m_CollisionState;
};

#endif // BASEGAMEENTITY_H
