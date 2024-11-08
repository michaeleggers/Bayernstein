#ifndef _GAME_H_
#define _GAME_H_

#include <string>

#include "CWorld.h"
#include "Entity/Door/g_door.h"
#include "Entity/Enemy/g_enemy.h"
#include "Entity/Player/g_player.h"
#include "Entity/entity_manager.h"
#include "camera.h"
#include "hkd_interface.h"
#include "irender.h"

class Game {
  public:
    Game(std::string exePath, hkdInterface *interface);

    void Init();
    bool RunFrame(double dt);
    void Shutdown();

  private:
    hkdInterface                *m_Interface;
    std::string                 m_ExePath;

    Player* m_pPlayerEntity;
    Player* m_pDebugPlayerEntity; // Entity we can fly around with
    Enemy*  m_pEnemyEntity;
    EntityManager* m_pEntityManager;

    Camera m_Camera;
    Camera m_FollowCamera;

    std::vector<HKD_Model*> m_Models;
    CFont* m_ConsoleFont;
    CFont* m_ConsoleFont30;
    Box m_SkyBox{};

    double m_AccumTime;

    CWorld m_World;
};

#endif
