//
// Created by me on 10/21/24.
//

#ifndef _DOOR_STATES_H_
#define _DOOR_STATES_H_

#include "../../FSM/istate.h"
#include "g_door.h"

class DoorIdle : public State<Door> {
  private:
    DoorIdle() = default;
    // copy ctor and assignment should be private
    DoorIdle(const DoorIdle&);
    DoorIdle& operator=(const DoorIdle&);

  public:
    // this is a singleton
    static DoorIdle* Instance();

    void Enter(Door* pDoor) override;
    void Execute(Door* pDoor) override;
    void Exit(Door* pDoor) override;
    bool OnMessage(Door* agent, const Telegram& telegram) override;
};

#endif // _DOOR_STATES_H_

