//
// Created by me on 10/14/24.
//

#ifndef _FP_PLAYER_H_
#define _FP_PLAYER_H_

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../collision.h"
#include "../../r_model.h"
#include "../../input_receiver.h"
#include "../moving_entity.h"


class FirstPersonPlayer : public MovingEntity, public IInputReceiver {

  public:
    explicit FirstPersonPlayer(glm::vec3 initialPosition);
    ~FirstPersonPlayer() override {
        delete m_pStateMachine;
    }

    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    void HandleInput() override;
    
    void UpdateCamera(Camera* camera);
    void UpdatePosition(glm::vec3 newPosition) override;

    [[nodiscard]] StateMachine<FirstPersonPlayer>* GetFSM() const {
        return m_pStateMachine;
    }

    EllipsoidCollider GetEllipsoidCollider() const override;

    HKD_Model* GetModel();

  public:
    bool CanAttack() {
        double currentTime = Clock->GetTime();
        if ( currentTime >= m_LastAttack + m_AttackDelay ) {
            m_LastAttack = currentTime;
            return true;
        }
        return false;
    }
  
private:
    StateMachine<FirstPersonPlayer>* m_pStateMachine;
    double m_AttackDelay = 100;
    double m_LastAttack = 0;

    HKD_Model m_Model;
    // moving members

private:
    glm::vec3 m_Forward, m_Side;
    AnimState m_AnimationState;
    EllipsoidCollider m_EllipsoidCollider;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdatePlayerModel();
};

#endif // _FP_PLAYER_H_

