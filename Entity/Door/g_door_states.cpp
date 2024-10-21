//
// Created by me on 10/21/24.
//

#include "g_door_states.h"

#include <stdio.h>
#include "../../utils.h"
#include "../Message/message_type.h"

// Door is closed

DoorClosed *DoorClosed::Instance() {
    static DoorClosed instance;

    return &instance;
}

void DoorClosed::Enter(Door *pDoor) {
    printf("Door entered Closed State\n");
}

void DoorClosed::Execute(Door *pDoor) {
    //printf("Door is executing Closed State\n");
}

void DoorClosed::Exit(Door *pDoor) {
    printf("Door is exiting Closed State\n");
}

bool DoorClosed::OnMessage(Door *agent, const Telegram &telegram) { 
    printf("Door received telegram: %s\n", MessageToString(telegram.Message).c_str());

    switch (telegram.Message) {
        case message_type::Collision: {
            agent->GetFSM()->ChangeState(DoorOpening::Instance());
            return true;
        } break;
        default: {
            printf("Door is not interested in this message.\n");
        }
    }

    return false; 
}


// Door is opening


DoorOpening *DoorOpening::Instance() {
    static DoorOpening instance;

    return &instance;
}

void DoorOpening::Enter(Door *pDoor) {
    printf("Door entered Opening State\n");
}

void DoorOpening::Execute(Door *pDoor) {
    printf("Door is executing Opening State\n");

    // TODO: Check if door has reached its opened state.
    // If so switch to the opened state.
        

}

void DoorOpening::Exit(Door *pDoor) {
    printf("Door is exiting Opening State\n");
}

bool DoorOpening::OnMessage(Door *agent, const Telegram &telegram) { 

    return false; 
}

