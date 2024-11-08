#ifndef _FOLLOW_CAMERA_H_
#define _FOLLOW_CAMERA_H_

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../collision.h"
#include "../base_game_entity.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

class CFollowCamera : public BaseGameEntity {

public:
    CFollowCamera( const int id, glm::vec3 pos = glm::vec3(0.0f), BaseGameEntity* target = nullptr);
    ~CFollowCamera();

    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;

private:
    Camera m_Camera;
    BaseGameEntity* m_Target = nullptr;
};

#endif


