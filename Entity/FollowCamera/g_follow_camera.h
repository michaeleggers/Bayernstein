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

/*
 * This entity stores a regular Camera. But it also needs
 * another target entity to follow. When the target entity
 * moves, this entity (the camera) follows it. It is not
 * controllable by user input (Actually, it is because
 * you could always just use the low-level raw SDL inputs
 * from input.cpp, but responding to high-level inputmappings
 * is not possible as it doesn't implement the IInputReceiver).
*/
class CFollowCamera : public BaseGameEntity {

  public:
    CFollowCamera(BaseGameEntity* target = nullptr);
    ~CFollowCamera();

    void PostCollisionUpdate() override;
    bool HandleMessage(const Telegram& telegram) override;

    void SetTarget(BaseGameEntity* target);

    Camera m_Camera;

  private:
    BaseGameEntity* m_Target = nullptr;
};

#endif
