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
    //printf("Door is executing Opening State\n");

    // TODO: Maybe it is a good idea to update
    // this at a fixed interval since eg. moving
    // two doors with the same properties will
    // end up at slightly different positions due
    // to variations of the deltaTime and floating
    // point rounding errors.
    

    // Check if door has reached its opened state.
    // If so switch to the opened state.
    //printf("dt: %f\n", GetDeltaTime());
    pDoor->m_CurrentDistance += pDoor->m_Speed * GetDeltaTime()/1000.0;
    /*printf("current distance: %f\n", pDoor->m_CurrentDistance);*/
    if ( pDoor->m_CurrentDistance >= pDoor->m_Distance ) {
        pDoor->GetFSM()->ChangeState(DoorOpened::Instance());
        
        return;
    }

    // Open the door.
    std::vector<TriPlane>& triPlanes = pDoor->TriPlanes();
    for (int i = 0; i < triPlanes.size(); i++) {
        Tri& tri = triPlanes[ i ].tri;
        for (int j = 0; j < 3; j++) {
            Vertex& v = tri.vertices[ j ];
            v.pos.z += pDoor->m_Speed * GetDeltaTime()/1000.0;
        }
    }
}

void DoorOpening::Exit(Door *pDoor) {
    printf("Door is exiting Opening State\n");
}

bool DoorOpening::OnMessage(Door *agent, const Telegram &telegram) { 

    return false; 
}


// Door is open 


DoorOpened *DoorOpened::Instance() {
    static DoorOpened instance;

    return &instance;
}

void DoorOpened::Enter(Door *pDoor) {
    printf("Door entered Opened State\n");
}

void DoorOpened::Execute(Door *pDoor) {
    //printf("Door is executing Opened State\n");

}

void DoorOpened::Exit(Door *pDoor) {
    //printf("Door is exiting Opened State\n");
}

bool DoorOpened::OnMessage(Door *agent, const Telegram &telegram) { 

    return false; 
}
