//
// Created by benek on 10/15/24.
//

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H
#include "base_game_entity.h"
#include <map>

class EntityManager {
  private:
	typedef std::map<int, BaseGameEntity*> EntityMap;

  private:
	EntityMap m_EntityMap;
	EntityManager() = default;

  public:
	// copy ctor and assignment should be private
	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;
	~EntityManager();

	static EntityManager* Instance();

	void RegisterEntity(BaseGameEntity* NewEntity);
	void KillEntities();

	// returns a pointer to the entity with the ID given as a parameter
	[[nodiscard]] BaseGameEntity* GetEntityFromID(int id) const;

	// this method removes the entity from the list
	void RemoveEntity(const BaseGameEntity* pEntity);

	void UpdateEntities();
};

#endif // ENTITYMANAGER_H
