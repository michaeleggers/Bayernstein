//
// Created by me on 10/21/24.
//

#ifndef _DOOR_H_
#define _DOOR_H_

#include <vector>

#include "../../Clock/clock.h"
#include "../../FSM/state_machine.h"
#include "../../Message/message_dispatcher.h"
#include "../base_game_entity.h"
#include "../../map_parser.h"
#include "../../r_common.h"

class Door : public BaseGameEntity {
private:

    StateMachine<Door>* m_pStateMachine;

    // TODO: I don't think that a door should hold on
    // to their geometry but rather point into a large
    // triangle buffer.
    std::vector<TriPlane> m_TriPlanes;

    // If door is in open state this is the dalay to close
    // if no agent is around.
    double m_ClosingDelayInMs = 100.0;

    // Speed the door opens/closes with
    double m_SpeedInMs = 500.0;

    // This is the angle in degrees the door slides to when opening.
    // On closing it is just the opposite direction.
    double m_Angle = 0.0;

public:

    explicit Door(const int id, std::vector<Property>& properties, std::vector<Brush>& brushes);
    
    void Update() override;

    ~Door() override {
        delete m_pStateMachine;
    }

    StateMachine<Door>* GetFSM() const {
        return m_pStateMachine;
    }

    bool HandleMessage(const Telegram& telegram) override;

    std::vector<TriPlane>& TriPlanes() {
        return m_TriPlanes;
    }

};

#endif // _DOOR_H_

