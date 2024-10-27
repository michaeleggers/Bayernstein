//
// Created by benek on 10/14/24.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../collision.h"
#include "../../r_model.h"
#include "../base_game_entity.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

class Player : public BaseGameEntity {
  private:
    StateMachine<Player>* m_pStateMachine;
    double m_AttackDelay = 100;
    double m_LastAttack = 0;

    HKD_Model m_Model;
    // moving members
  private:
    glm::vec3 m_Velocity;
    glm::vec3 m_Forward, m_Side;
    float m_RotationAngle;
    AnimState m_AnimationState;
    EllipsoidCollider m_EllipsoidCollider;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdatePlayerModel();

  public:
    explicit Player(const int id, glm::vec3 initialPosition);

    ~Player() override {
        delete m_pStateMachine;
    }

    void Update() override;
    void UpdateCamera(Camera* camera);
    void UpdatePosition(glm::vec3 newPosition);

    [[nodiscard]] StateMachine<Player>* GetFSM() const {
        return m_pStateMachine;
    }

    bool HandleMessage(const Telegram& telegram) override;

    EllipsoidCollider GetEllipsoidCollider() const;
    HKD_Model* GetModel();
    glm::vec3 GetVelocity() const;

  public:
    bool CanAttack() {
        double currentTime = Clock->GetTime();
        if (currentTime >= m_LastAttack + m_AttackDelay) {
            m_LastAttack = currentTime;
            return true;
        }
        return false;
    }
};

#endif // PLAYER_H
