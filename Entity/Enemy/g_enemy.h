//
// Created by benek on 10/14/24.
//

#ifndef ENEMY_H
#define ENEMY_H

#include "../../FSM/state_machine.h"
#include "../../collision.h"
#include "../../r_model.h"
#include "../Path/path.h"
#include "../moving_entity.h"
#include "../steering_behaviour.h"
#include "../base_game_entity.h"
#include "../../input_receiver.h"
#include "../../map_parser.h"

class Enemy : public MovingEntity {
  public:
    void Update() override;

    explicit Enemy(const std::vector<Property>& properties);

    ~Enemy() override {
        delete m_pStateMachine;
        delete m_pSteeringBehaviour;
    }
   
    [[nodiscard]] StateMachine<Enemy>* GetFSM() const {
        return m_pStateMachine;
    }

    bool HandleMessage(const Telegram& message) override;
    EllipsoidCollider GetEllipsoidCollider() const override;
    HKD_Model* GetModel();
    void UpdatePosition(glm::vec3 newPosition) override;

    void SetSeekTarget(BaseGameEntity* target) {
        m_pSteeringBehaviour->SetTargetAgent(target);
        m_pSteeringBehaviour->SeekOn();
    }

    void SetFleeTarget(BaseGameEntity* target) {
        m_pSteeringBehaviour->SetTargetAgent(target);
        m_pSteeringBehaviour->FleeOn();
    }

    void SetArriveTarget(BaseGameEntity* target) {
        m_pSteeringBehaviour->SetTargetAgent(target);
        m_pSteeringBehaviour->ArriveOn();
    }
    void SetFollowPath(PatrolPath* path) {
        m_pSteeringBehaviour->SetFollowPath(path);
        m_pSteeringBehaviour->FollowPathOn();
    }

  public:
    bool DecreaseHealth(double amount) {
        m_Health = m_Health - amount;
        return true;
    }

    bool IsDead() {
        return m_Health <= 0.0;
    }
  
private:
    StateMachine<Enemy>* m_pStateMachine;
    SteeringBehaviour* m_pSteeringBehaviour;
    double m_Health = 100;

    HKD_Model m_Model;
    // moving members

private:
    AnimState m_AnimationState;
    EllipsoidCollider m_EllipsoidCollider;

    void LoadModel(const char* path, glm::vec3 initialPosition);
    void UpdateEnemyModel();

};

#endif // ENEMY_H

