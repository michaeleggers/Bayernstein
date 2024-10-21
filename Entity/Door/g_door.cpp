//
// Created by me on 10/21/24.
//

#include "g_door.h"

#include "../input.h"
#include "g_door_states.h"
#include <SDL.h>

void Door::Update() {


	m_pStateMachine->Update();
}

Door::Door() : BaseGameEntity(0), m_pStateMachine(nullptr) {
	m_pStateMachine = new StateMachine(this);
	m_pStateMachine->SetCurrentState(DoorIdle::Instance());
}

bool Door::HandleMessage(const Telegram& telegram) {
	return m_pStateMachine->HandleMessage(telegram);
}

