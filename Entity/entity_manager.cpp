//
// Created by benek on 10/15/24.
//

#include "entity_manager.h"
#include "Player/g_player.h"
#include <assert.h>

//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
EntityManager* EntityManager::Instance() {
	static EntityManager instance;

	return &instance;
}

EntityManager::~EntityManager() {
	for (auto [id, entity] : m_EntityMap) {
		delete entity;
	}
}

void EntityManager::RegisterEntity(BaseGameEntity* NewEntity, CWorld* world) {
	NewEntity->SetWorld(world);
	m_EntityMap.insert(std::make_pair(NewEntity->ID(), NewEntity));
}

BaseGameEntity* EntityManager::GetEntityFromID(int id) const {
	EntityMap::const_iterator entity = m_EntityMap.find(id);

	// assert that the entity is a member of the map
	assert((entity != m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

	return entity->second;
}
Player* EntityManager::GetPlayerEntity() const {
	return (Player*)GetEntityFromID(0);
}

void EntityManager::RemoveEntity(const BaseGameEntity* pEntity) {
	m_EntityMap.erase(m_EntityMap.find(pEntity->ID()));
}

void EntityManager::UpdateEntities(double dt) {
	for (auto [id, entity] : m_EntityMap) {
		entity->Update(dt);
	}
}
