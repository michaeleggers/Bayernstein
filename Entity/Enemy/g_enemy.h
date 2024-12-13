
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H

#include "../../FSM/state_machine.h"
#include "../../collision.h"
#include "../../map_parser.h"
#include "../../r_model.h"
#include "../Path/path.h"
#include "../Player/g_player.h"
#include "../moving_entity.h"
#include "../steering_behaviour.h"
#include "g_enemy_states.h"

std::vector<Tri> GenerateVisionCone(glm::vec3 position,
                                    glm::vec3 direction,
                                    glm::vec3 up,
                                    float     angle,
                                    float     range,
                                    float     height,
                                    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0));

class Enemy : public MovingEntity {
  public:
    void Update() override;

    explicit Enemy(const std::vector<Property>& properties);

    ~Enemy() override {
        delete m_pStateMachine;
        delete m_pSteeringBehaviour;
        delete m_Path;
    }

    [[nodiscard]] StateMachine<Enemy>* GetFSM() const {
        return m_pStateMachine;
    }

    bool              HandleMessage(const Telegram& telegram) override;
    EllipsoidCollider GetEllipsoidCollider() const override;
    HKD_Model*        GetModel();
    void              UpdatePosition(glm::vec3 newPosition) override;

    void Wander() {
        m_pStateMachine->ChangeState(EnemyWander::Instance());
    }

    void Idle() {
        m_pStateMachine->ChangeState(EnemyIdle::Instance());
    }

    void Patrol() {
        assert(m_Path != nullptr && !m_Target.empty() && "Enemy has no path or target!");
        m_pStateMachine->ChangeState(EnemyPatrol::Instance());
    }

    void SetPatrolPath(PatrolPath* path) {
        m_Path = path;
        printf("z of entity: %f, z of points %f\n", m_Position.z, path->GetPoints()[ 0 ].position.z);
        m_Path->m_OffsetToEntity = m_Position.z - path->GetPoints()[ 0 ].position.z;
        m_pSteeringBehaviour->SetFollowPath(m_Path);
    }

  public:
    bool DecreaseHealth(double amount) {
        m_Health = m_Health - amount;
        return true;
    }

    bool IsDead() {
        return m_Health <= 0.0;
    }

    SteeringBehaviour* m_pSteeringBehaviour;
    std::vector<Tri>   m_vision_cone;
    bool               m_PlayerDetected    = false;
    bool               m_PlayerInRange     = false;
    bool               m_PlayerOnSameLevel = false;
    bool               m_PlayerInAngle     = false;
    bool               m_PlayerInSight     = false;
    float              m_range             = 200;
    float              m_vision_angle      = 45;
    float              m_vision_height     = 50;
    Player*            m_pPlayerEnity;

    void RegisterPlayer(Player* pPlayer) {
        m_pPlayerEnity = pPlayer;
    }

  private:
    StateMachine<Enemy>* m_pStateMachine;
    double               m_Health = 100;

    // FIX: Those should be components for next milestone.
    HKD_Model   m_Model;
    PatrolPath* m_Path;
    // moving members

  private:
    AnimState         m_AnimationState;
    EllipsoidCollider m_EllipsoidCollider;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdateEnemyModel();
};

#endif // ENEMY_H
