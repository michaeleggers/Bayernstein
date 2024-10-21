//
// Created by me on 10/21/24.
//

#ifndef _DOOR_STATES_H_
#define _DOOR_STATES_H_

#include "../../FSM/istate.h"
#include "g_door.h"

class DoorClosed : public State<Door> {
  private:
    DoorClosed() = default;
    // copy ctor and assignment should be private
    DoorClosed(const DoorClosed&);
    DoorClosed& operator=(const DoorClosed&);

  public:
    // this is a singleton
    static DoorClosed* Instance();

    void Enter(Door* pDoor) override;
    void Execute(Door* pDoor) override;
    void Exit(Door* pDoor) override;
    bool OnMessage(Door* agent, const Telegram& telegram) override;
};

class DoorOpening : public State<Door> {
  private:
    DoorOpening() = default;
    // copy ctor and assignment should be private
    DoorOpening(const DoorOpening&);
    DoorOpening& operator=(const DoorOpening&);

  public:
    // this is a singleton
    static DoorOpening* Instance();

    void Enter(Door* pDoor) override;
    void Execute(Door* pDoor) override;
    void Exit(Door* pDoor) override;
    bool OnMessage(Door* agent, const Telegram& telegram) override;
};

#endif // _DOOR_STATES_H_


