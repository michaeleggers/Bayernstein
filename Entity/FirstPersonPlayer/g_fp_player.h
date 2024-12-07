//
// Created by me on 11/23/24.
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
    // Movement related values
    static constexpr float MAX_VELOCITY             = 350.0f;
    static constexpr float RUN_VELOCITY             = 300.0f;
    static constexpr float WALK_FACTOR              = 0.3f;
    static constexpr float MAX_MOUSE_LOOK_DEGREES   = 89.0f;

    // TODO: What are the correct units to use?
    // NOTE: Use this (for now) to control the gravity
    // and *not* m_Gravity inside CWorld! m_Gravity of
    // CWorld is only really used for stair climbing 
    // at this moment.
    static constexpr float GRAVITY_ACCELERATION     = 2.5f;
    static constexpr float JUMPING_MOMENTUM         = 1000.0f;

    // How much can you move when in air?
    // 1.0: maximum friction => Not able to change direction in air.
    // 0.0: no friction => Fully able to change direction in air.
    static constexpr float IN_AIR_FRICTION          = 0.1f;

    explicit FirstPersonPlayer(glm::vec3 initialPosition);
    
    ~FirstPersonPlayer() override {
        delete m_pStateMachine;
    }

    void PostCollisionUpdate() override;
    bool HandleMessage(const Telegram& telegram) override;
    void HandleInput() override;
    
    void UpdateCamera(Camera* camera);
    void UpdatePosition(glm::vec3 newPosition) override;

    [[nodiscard]] StateMachine<FirstPersonPlayer>* GetFSM() const {
        return m_pStateMachine;
    }

    EllipsoidCollider* GetEllipsoidColliderPtr() override;

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

    Camera& GetCamera() {
        return m_Camera;
    }
  
private:
    StateMachine<FirstPersonPlayer>* m_pStateMachine;
    double m_AttackDelay = 100;
    double m_LastAttack = 0;

    HKD_Model m_Model;
    // moving members

private:
    glm::vec3           m_Forward, m_Side;
    AnimState           m_AnimationState;
    Camera              m_Camera;
    float               m_Pitch = 0.0f; // The thing when the camera rotates around its side axis
    float               m_Yaw   = 0.0f; // The thing when the camera rotates around the world up axis
    float               m_LookSpeed = 30.0f;
    int                 m_MouseX = 0;
    int                 m_MouseY = 0;
    int                 m_MousePrevX = 0;
    int                 m_MousePrevY = 0;
    bool                m_IsJumping = false;
    glm::vec3           m_Momentum = glm::vec3(0.0f);
    glm::vec3           m_Dir = glm::vec3(0.0f);
    glm::vec3           m_FlyMomentum = glm::vec3(0.0f);
    EntityCollisionState m_PrevCollisionState;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdatePlayerModel();
};

#endif // _FP_PLAYER_H_

