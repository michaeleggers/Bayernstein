#include "g_fly_camera.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "../../input_handler.h"
#include "../../input_receiver.h"
#include "../../input.h"
#include "../../utils/utils.h"

CFlyCamera::CFlyCamera(glm::vec3 pos)
    : BaseGameEntity(ET_FLY_CAMERA) {

    m_Camera = Camera(pos);
    m_Position = pos;
}

CFlyCamera::~CFlyCamera() {
}

void CFlyCamera::Update() {
    
}

bool CFlyCamera::HandleMessage(const Telegram& telegram) {
    return false;
}

void CFlyCamera::HandleInput() {
    ButtonState forward = CHECK_ACTION("forward");
    ButtonState back    = CHECK_ACTION("back");
    ButtonState left    = CHECK_ACTION("left");
    ButtonState right   = CHECK_ACTION("right");
    ButtonState look    = CHECK_ACTION("look");

    float dt = (float)GetDeltaTime();
    if (forward == ButtonState::PRESSED) {
        m_Camera.Pan(dt*m_Camera.m_Forward);
    }
    if (back == ButtonState::PRESSED) {
        m_Camera.Pan(dt*-m_Camera.m_Forward);
    }
    if (left == ButtonState::PRESSED) {
        m_Camera.Pan(-dt*m_Camera.m_Side);
    }
    if (right == ButtonState::PRESSED) {
        m_Camera.Pan(dt*m_Camera.m_Side);
    }

    if (look == ButtonState::WENT_DOWN) {
        const MouseMotion mouseMotion = GetMouseMotion();
        m_MouseX = mouseMotion.current.x;
        m_MouseY = mouseMotion.current.y;
    }
    else if (look == ButtonState::PRESSED) {
        const MouseMotion mouseMotion = GetMouseMotion();
        m_MousePrevX = m_MouseX;
        m_MousePrevY = m_MouseY;
        m_MouseX = mouseMotion.current.x;
        m_MouseY = mouseMotion.current.y;
        int dX = m_MouseX - m_MousePrevX;
        int dY = m_MouseY - m_MousePrevY;
        m_Camera.RotateAroundWorldUp( -m_LookSpeed*(float)dX ); 
        m_Camera.RotateAroundSide( -m_LookSpeed*(float)dY );
    }
    else if (look == ButtonState::WENT_UP) {
        m_MouseX = 0;
        m_MouseY = 0;
        m_MousePrevX = 0;
        m_MousePrevY = 0;
    }

    m_Position = m_Camera.m_Pos;
}


