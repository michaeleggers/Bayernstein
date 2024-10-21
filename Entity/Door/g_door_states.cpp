//
// Created by me on 10/21/24.
//

#include "g_door_states.h"

#include <stdio.h>
#include "../../utils.h"
#include "../Message/message_type.h"

DoorIdle *DoorIdle::Instance() {
    static DoorIdle instance;

    return &instance;
}

void DoorIdle::Enter(Door *pDoor) {
    printf("Door entered Idle State\n");
}

void DoorIdle::Execute(Door *pDoor) {printf("Door is executing Idle State\n"); }
void DoorIdle::Exit(Door *pDoor) {printf("Door is exiting Idle State\n"); }
bool DoorIdle::OnMessage(Door *agent, const Telegram &telegram) { return false; }

