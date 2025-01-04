//
// Created by me on 04/01/25.
//

#ifndef _WEAPON_H_
#define _WEAPON_H_

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "../../Message/message_dispatcher.h"
#include "../../r_model.h"
#include "../base_game_entity.h"

class Weapon : public BaseGameEntity
{

  public:
    explicit Weapon(const std::vector<Property>& properties);

    ~Weapon() override {}

    bool HandleMessage(const Telegram& telegram) override;

    void UpdatePosition(glm::vec3 newPosition) override;

    void PostCollisionUpdate() override {};

    EllipsoidCollider* GetEllipsoidColliderPtr() override;

    HKD_Model* GetModel();

  public:
  private:
    HKD_Model m_Model;

  private:
    void LoadModel(const char* path, glm::vec3 initialPosition);
};

#endif
