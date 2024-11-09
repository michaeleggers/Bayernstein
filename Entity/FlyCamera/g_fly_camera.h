#ifndef _FLY_CAMERA_H_
#define _FLY_CAMERA_H_

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../../camera.h"
#include "../../collision.h"
#include "../../input_receiver.h"
#include "../base_game_entity.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

class CFlyCamera : public BaseGameEntity, public IInputReceiver {

public:
    CFlyCamera() = delete;
    CFlyCamera(const int id, glm::vec3 pos = glm::vec3(0.0f)); 
    ~CFlyCamera();

    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    void HandleInput() override;
    
    Camera m_Camera;

private:
};

#endif




