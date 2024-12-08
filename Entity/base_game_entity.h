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
#include "../utils/utils.h"
#include "../map_parser.h"


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

    template<typename T>
    bool GetProperty(const std::vector<Property>& properties, std::string propName, T* value) {
        for ( const Property& property : properties ) {
            if ( property.key == propName ) {
                *value = ParseProperty<T>(property.value);
                return true;
            }
        }

        return false;
    }

    template<typename T>
    T ParseProperty(const std::string& sValue);

    template<>
    glm::vec3 ParseProperty<glm::vec3>(const std::string& sValue) {
        std::vector<float> origin = ParseValues<float>(sValue);
        return glm::vec3( origin[0], origin[1], origin[2] );
    }

    template<>
    std::string ParseProperty<std::string>(const std::string& sValue) {
        return sValue;
    }

    template<>
    float ParseProperty<float>(const std::string& sValue) {
        std::vector<float> floats = ParseValues<float>(sValue);
        return floats[ 0 ];
    }
    
    template<>
    double ParseProperty<double>(const std::string& sValue) {
        std::vector<double> floats = ParseValues<double>(sValue);
        return floats[ 0 ];
    }
    
    template<>
    int ParseProperty<int>(const std::string& sValue) {
        std::vector<int> ints = ParseValues<int>(sValue);
        return ints[ 0 ];
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
    // We should think about an Actor-Component model. Maybe
    // overkill, but at least discuss it.
    virtual EllipsoidCollider GetEllipsoidCollider() const {
        return {};
    };
    
    glm::vec3   m_Position = glm::vec3(0.0f);
    glm::vec3   m_Velocity = glm::vec3(0.0f); // TODO: Actually make use of it and remove from subclasses!
    float       m_RotationAngle = 0.0f; // TODO: Should be a quaternion called m_Orientation.
    std::string m_Target = "";
};

#endif // BASEGAMEENTITY_H

