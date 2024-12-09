
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H

#include "../../FSM/state_machine.h"
#include "../../collision.h"
#include "../../input_receiver.h"
#include "../../map_parser.h"
#include "../../r_model.h"
#include "../Path/path.h"
#include "../base_game_entity.h"
#include "../moving_entity.h"
#include "../steering_behaviour.h"
#include "g_enemy_states.h"

class Enemy : public MovingEntity {
  public:

    explicit Enemy(const std::vector<Property>& properties);

    ~Enemy() override {
        delete m_pStateMachine;
        delete m_pSteeringBehaviour;
        delete m_Path; 
    }
   
    [[nodiscard]] StateMachine<Enemy>* GetFSM() const {
        return m_pStateMachine;
    }

    bool                HandleMessage(const Telegram& message) override;
    EllipsoidCollider*  GetEllipsoidColliderPtr() override; 
    void                PreCollisionUpdate() override;
    void                PostCollisionUpdate() override;
    void                UpdatePosition(glm::vec3 newPosition) override;
    
    HKD_Model*          GetModel();

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

  private:
    StateMachine<Enemy>* m_pStateMachine;
    double               m_Health = 100;

    // FIX: Those should be components for next milestone.
    HKD_Model   m_Model;
    PatrolPath* m_Path;
    // moving members

private:
    AnimState m_AnimationState;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdateEnemyModel();
};

#endif // ENEMY_H
