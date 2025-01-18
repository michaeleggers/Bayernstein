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

    /**
     * Triggers the weapon (i.e. shoot/attack), but only if it is allowed to according to its properies.
     * @returns `true` if the requested fire was allowed/executed, `false` otherwise
     */
    bool Fire();

  public:
  private:
    HKD_Model m_Model;

    SoLoud::AudioSource* m_SfxGunshot;
    SoLoud::AudioSource* m_SfxReload;

    /** The size of the weapon's magazine, i.e. maximum number of available shots before reload. */
    int m_MagSize;
    /** The time delay between individual shots in milliseconds. */
    double m_FireRate;
    /** The duration of a reload in millisecons. */
    double m_ReloadTime;
    /** The number of remaining shots until a reload is necessary. */
    int m_RoundsRemaining;
    /** Counter in milliseconds to check if a shot is allowed. */
    double m_TimeElapsed = 0.0;

  private:
    void LoadModel(const char* path, glm::vec3 initialPosition);
};

#endif
