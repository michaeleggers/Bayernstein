//
// Created by me on 04/01/25.
//

#ifndef _FP_PLAYER_H_
#define _FP_PLAYER_H_

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../collision.h"
#include "../../r_model.h"

class Weapon : public BaseGameEntity
{

  public:
    explicit Weapon(const std::vector<Property>& properties);

    ~Weapon() override
    {
        delete m_pStateMachine;
    }

    bool HandleMessage(const Telegram& telegram) override;

    void UpdateCamera(Camera* camera);
    void UpdatePosition(glm::vec3 newPosition) override;

    EllipsoidCollider* GetEllipsoidColliderPtr() override;

    HKD_Model* GetModel();

  public:
    Camera& GetCamera()
    {
        return m_Camera;
    }

  private:
    HKD_Model m_Model;

  private:
    glm::vec3 m_Forward, m_Side;
    AnimState m_AnimState;
    Camera    m_Camera;

    void LoadModel(const char* path, glm::vec3 initialPosition);
};

#endif
