
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H

#include "soloud.h"

#include "../../FSM/state_machine.h"
#include "../../collision.h"
#include "../../input_receiver.h"
#include "../../map_parser.h"
#include "../../r_model.h"
#include "../../utils/quick_math.h"
#include "../Path/path.h"
#include "../base_game_entity.h"
#include "../moving_entity.h"
#include "../steering_behaviour.h"
#include "g_enemy_states.h"

class Enemy : public MovingEntity
{
  public:
    explicit Enemy(const std::vector<Property>& properties);

    ~Enemy() override
    {
        if ( m_pStateMachine )
        {
            delete m_pStateMachine;
        }
        if ( m_pSteeringBehaviour )
        {
            delete m_pSteeringBehaviour;
        }
        if ( m_pPath )
        {
            delete m_pPath;
        }
    }

    [[nodiscard]] StateMachine<Enemy>* GetFSM() const
    {
        return m_pStateMachine;
    }

    bool               HandleMessage(const Telegram& message) override;
    EllipsoidCollider* GetEllipsoidColliderPtr() override;
    void               PreCollisionUpdate() override;
    void               PostCollisionUpdate() override;
    void               UpdatePosition(glm::vec3 newPosition) override;

    HKD_Model* GetModel();

    void Wander()
    {
        m_pStateMachine->ChangeState(EnemyWander::Instance());
    }

    void Idle()
    {
        m_pStateMachine->ChangeState(EnemyIdle::Instance());
    }

    void Patrol()
    {
        assert(m_pPath != nullptr && !m_Target.empty() && "Enemy has no path or target!");
        m_pStateMachine->ChangeState(EnemyPatrol::Instance());
    }

    void SetPatrolPath(PatrolPath* path)
    {
        m_pPath = path;
        printf("z of entity: %f, z of points %f\n", m_Position.z, path->GetPoints()[ 0 ].position.z);
        m_pPath->m_OffsetToEntity = m_Position.z - path->GetPoints()[ 0 ].position.z;
        m_pSteeringBehaviour->SetFollowPath(m_pPath);
    }

    void SetTarget(BaseGameEntity* pEntity)
    {
        m_pTargetEntity = pEntity;
    }

  public:
    bool DecreaseHealth(double amount)
    {
        m_Health = m_Health - amount;
        return true;
    }

    bool IsDead()
    {
        return m_Health <= 0.0;
    }
    SteeringBehaviour* m_pSteeringBehaviour;

    float m_ProjDistance;
    float m_AspectRatio;
    float m_Near;
    float m_Far;

    // FIX: Those should be components for next milestone.
    HKD_Model   m_Model;
    PatrolPath* m_pPath;

  private:
    StateMachine<Enemy>* m_pStateMachine;
    double               m_Health = 100;

    SoLoud::AudioSource* m_SfxFootsteps;
    SoLoud::handle       m_FootstepsHandle = 0;

    BaseGameEntity* m_pTargetEntity;

  private:
    AnimState m_AnimationState;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdateEnemyModel();
};

#endif // ENEMY_H
