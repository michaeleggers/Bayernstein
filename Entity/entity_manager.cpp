//
// Created by benek on 10/15/24.
//

#include "entity_manager.h"
#include <assert.h>

//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
EntityManager *EntityManager::Instance() {
	static EntityManager instance;

	return &instance;
}

EntityManager::~EntityManager() {
    // TODO: Could be deleted? (We might have use for this later, though).
}

void EntityManager::RegisterEntity(BaseGameEntity *NewEntity) {
	m_EntityMap.insert(std::make_pair(NewEntity->ID(), NewEntity));
}

void EntityManager::KillEntities() {
	for (auto [id, entity] : m_EntityMap) {
		delete entity;
	}
}

BaseGameEntity *EntityManager::GetEntityFromID(int id) const {
	EntityMap::const_iterator entity = m_EntityMap.find(id);

	// assert that the entity is a member of the map
	assert((entity != m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

	return entity->second;
}

void EntityManager::RemoveEntity(const BaseGameEntity *pEntity) { m_EntityMap.erase(m_EntityMap.find(pEntity->ID())); }

void EntityManager::UpdateEntities() {
	for (auto [id, entity] : m_EntityMap) {
		entity->Update();
	}
}
