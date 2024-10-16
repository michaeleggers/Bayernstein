//
// Created by benek on 10/14/24.
//

#include "BaseGameEntity.h"
#include <cassert>
#include <iostream>



int BaseGameEntity::m_iNextValidID = 0;

//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
void BaseGameEntity::SetID(int value) {
	// make sure the val is equal to or greater than the next available ID
	assert((value >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");
	std::cout << "value: " << value << " next valid: " << m_iNextValidID << "\n";

	m_ID = value;

	m_iNextValidID = m_ID + 1;
}
