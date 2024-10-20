//
// Created by benek on 10/14/24.
//

#ifndef BASEGAMEENTITY_H
#define BASEGAMEENTITY_H

#include "../Message/telegram.h"
#include "../CWorld.h"

class BaseGameEntity {
  private:
	int m_ID{};

	// this must be called within the constructor to make sure the ID is set
	// correctly. It verifies that the value passed to the method is greater
	// or equal to the next valid ID, before setting the ID and incrementing
	// the next valid ID
	void SetID(int value);

  public:
	CWorld* m_World;
	// this is the next valid ID. Each time a BaseGameEntity is instantiated
	// this value is updated
	static int m_iNextValidID;

	explicit BaseGameEntity(const int id) {
		SetID(id);
	}

	virtual ~BaseGameEntity() = default;

	// all entities must implement an update function
	virtual void Update(double dt) = 0;

	// all entities can communicate using messages. They are sent
	// using the MessageDispatcher singleton class
	virtual bool HandleMessage(const Telegram& telegram) = 0;

	[[nodiscard]] int ID() const {
		return m_ID;
	}

	void SetWorld(CWorld* world) {
		m_World = world;
	}
};

#endif // BASEGAMEENTITY_H
