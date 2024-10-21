//
// Created by me on 10/21/24.
//

#include "g_door.h"

#include "g_door_states.h"
#include "../../map_parser.h"
#include "../../polysoup.h"

#include <SDL.h>

void Door::Update() {

    m_pStateMachine->Update();
}

Door::Door(const int id, 
           std::vector<Property>& properties, 
           std::vector<Brush>& brushes) : BaseGameEntity(id), m_pStateMachine(nullptr) {

    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(DoorClosed::Instance());

    // Get all the door's properties from the MAP file.
    
    for (int i = 0; i < properties.size(); i++) {
        Property& prop = properties[ i ];
        if (prop.key == "angle") {
            // TODO: Parse the value (prop.value is always a string)
            // and assing to m_Angle;
        }
        else if (prop.key == "delay") {
        }
        // And so on...
    }

    // Get the brush (geometry) that defines this door
    // Assume just one brush for now... // TODO: Could be more brushes!
    std::vector<MapPolygon> mapPolys = createPolysoup( brushes[ 0 ] );
    std::vector<MapPolygon> mapTris = triangulate(mapPolys);
}

bool Door::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}

