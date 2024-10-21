//
// Created by me on 10/21/24.
//

#ifndef _DOOR_H_
#define _DOOR_H_

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../base_game_entity.h"

class Door : public BaseGameEntity {
  private:
    StateMachine<Door>* m_pStateMachine;
    double m_ClosingDelay = 100;

  public:
    void Update() override;
    explicit Door();

    ~Door() override {
        delete m_pStateMachine;
    }
    [[nodiscard]] StateMachine<Door>* GetFSM() const {
        return m_pStateMachine;
    }

    bool HandleMessage(const Telegram& telegram) override;

  public:
};

#endif // _DOOR_H_

