//
// Created by benek on 10/15/24.
//

#include "entity_manager.h"
#include <algorithm>
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
    // TODO: Could be deleted? (We might have use for this later, though).
}

void EntityManager::RegisterEntity(BaseGameEntity* pNewEntity) {
    pNewEntity->SetID(m_ID++);
    m_EntityMap.insert(std::make_pair(pNewEntity->ID(), pNewEntity));
}

void EntityManager::KillEntities() {
    for ( auto [ id, entity ] : m_EntityMap ) {
        delete entity;
    }
}

BaseGameEntity* EntityManager::GetEntityFromID(int id) const {
    EntityMap::const_iterator entity = m_EntityMap.find(id);

    // assert that the entity is a member of the map
    assert((entity != m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

    return entity->second;
}

Enemy* EntityManager::GetFirstEnemy() const {
    EntityMap::const_iterator entity = std::find_if(m_EntityMap.begin(), m_EntityMap.end(), [](const auto& entity) {
        return entity.second->Type() == ET_ENEMY;
    });

    // assert that the entity is a member of the map
    assert((entity != m_EntityMap.end()) && "<EntityManager::GetEnemies>: no Enemies found");

    return (Enemy*)entity->second;
}

void EntityManager::RemoveEntity(const BaseGameEntity* pEntity) {
    m_EntityMap.erase(m_EntityMap.find(pEntity->ID()));
}

void EntityManager::UpdateEntitiesPreCollision() {
    for ( auto [ id, entity ] : m_EntityMap ) {
        entity->PreCollisionUpdate();
    }
}

void EntityManager::UpdateEntitiesPostCollision() {
    for ( auto [ id, entity ] : m_EntityMap ) {
        entity->PostCollisionUpdate();
    }
}

std::vector<BaseGameEntity*> EntityManager::Entities() {
    std::vector<BaseGameEntity*> entities{};
    for ( auto [ id, entity ] : m_EntityMap ) {
        entities.push_back( entity );
    }

    return entities;
}

