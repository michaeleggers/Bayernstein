//
// Created by benek on 10/14/24.
//

#include "base_game_entity.h"
#include <assert.h>
#include <stdio.h>

//----------------------------- SetID -----------------------------------------
// Called by entity manager who is responsible to assign IDs.
//-----------------------------------------------------------------------------
void BaseGameEntity::SetID(int value) {
	m_ID = value;
}

