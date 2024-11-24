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

/* 
* This entity encapsulates a regular Camera. It implements
* the InputReceiver to get high-level input commands.
* You can fly around in the world with this camera by
* holding the right mouse-button and using WASD to 
* move forwards/backwards/left/right.
* TODO: We could think about having the camera itself being a
* BaseGameEntity. But I am not sure about this. We have to
* think about Entities a bit more anyways later.
*/
class CFlyCamera : public BaseGameEntity, public IInputReceiver {

public:
    CFlyCamera() = delete;
    CFlyCamera(glm::vec3 pos = glm::vec3(0.0f)); 
    ~CFlyCamera();

    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    void HandleInput() override;
    
    Camera m_Camera;
    float  m_LookSpeed = 0.01f;

private:
    int m_MouseX = 0;
    int m_MouseY = 0;
    int m_MousePrevX = 0;
    int m_MousePrevY = 0;
};

#endif




