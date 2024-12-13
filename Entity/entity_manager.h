//
// Created by benek on 10/15/24.
//

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H
#include "Enemy/g_enemy.h"
#include "base_game_entity.h"
#include <algorithm>
#include <map>

class EntityManager {
  private:
    typedef std::map<int, BaseGameEntity*> EntityMap;

  public:
    // copy ctor and assignment should be private
    EntityManager(const EntityManager&)            = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    ~EntityManager();

    static EntityManager* Instance();

    void RegisterEntity(BaseGameEntity* pNewEntity);
    void KillEntities();

    // returns a pointer to the entity with the ID given as a parameter
    [[nodiscard]] BaseGameEntity* GetEntityFromID(int id) const;
    [[nodiscard]] Enemy*          GetFirstEnemy() const;

    // this method removes the entity from the list
    void RemoveEntity(const BaseGameEntity* pEntity);

    void UpdateEntitiesPreCollision();
    void UpdateEntitiesPostCollision();

    // FIX: Slow!
    std::vector<BaseGameEntity*> Entities();

  private:
    EntityMap m_EntityMap;
    EntityManager() = default;
    int m_ID        = 0;
};

#endif // ENTITYMANAGER_H
