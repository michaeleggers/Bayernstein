//
// Created by benek on 10/26/24.
//

#ifndef STEERING_BEHAVIOUR_H
#define STEERING_BEHAVIOUR_H

#include <vector>

#include "../Path/path.h"
#include "./base_game_entity.h"
#include "moving_entity.h"

class SteeringBehaviour {
  public:
    enum summing_method {
        weighted_average,
        prioritized,
        dithered
    };

  public:
    SteeringBehaviour(MovingEntity* pEntity);
    glm::vec3 Calculate();
    void      SetTargetAgent(BaseGameEntity* pAgent) {
        m_pTargetAgent = pAgent;
    }
    void SetFollowPath(PatrolPath* pPath) {
        if ( pPath == nullptr ) {
            printf("SetFollowPath: pPath is nullptr! Ignore.\n");
            return;
        }
        m_pPath = pPath;
        // FollowWaypointsOn();
        // FollowPathOn();
    }

  public:
    void FleeOn() {
        m_Flags |= flee;
    }
    void SeekOn() {
        m_Flags |= seek;
    }
    void ArriveOn() {
        m_Flags |= arrive;
    }
    void WanderOn() {
        m_Flags |= wander;
    }
    void FollowPathOn() {
        m_Flags |= follow_path;
    }
    void FollowWaypointsOn() {
        m_Flags |= follow_waypoints;
    }

    void FleeOff() {
        if ( On(flee) ) m_Flags ^= flee;
    }
    void SeekOff() {
        if ( On(seek) ) m_Flags ^= seek;
    }
    void ArriveOff() {
        if ( On(arrive) ) m_Flags ^= arrive;
    }
    void WanderOff() {
        if ( On(wander) ) m_Flags ^= wander;
    }
    void FollowPathOff() {
        if ( On(follow_path) ) m_Flags ^= follow_path;
    }
    void FollowWaypointsOff() {
        if ( On(follow_waypoints) ) m_Flags ^= follow_waypoints;
    }

    bool isFleeOn() {
        return On(flee);
    }
    bool isSeekOn() {
        return On(seek);
    }
    bool isArriveOn() {
        return On(arrive);
    }
    bool isWanderOn() {
        return On(wander);
    }
    bool isFollowPathOn() {
        return On(follow_path);
    }
    bool isFollowWaypointsOn() {
        return On(follow_waypoints);
    }

  private:
    enum behavior_type {
        none   = 0x00000,
        seek   = 0x00002,
        flee   = 0x00004,
        arrive = 0x00008,
        wander = 0x00010,
        // cohesion = 0x00020,
        // separation = 0x00040,
        // allignment = 0x00080,
        // obstacle_avoidance = 0x00100,
        // wall_avoidance = 0x00200,
        follow_path      = 0x00400,
        follow_waypoints = 0x00800,
        // pursuit = 0x01000,
        // evade = 0x02000,
        // interpose = 0x04000,
        // hide = 0x08000,
        // flock = 0x10000,
        // offset_pursuit = 0x20000,
    };

  private:
    MovingEntity* m_pEntity;
    //the steering force created by the combined effect of all
    //the selected behaviors
    glm::vec3 m_SteeringForce;
    //the current target
    glm::vec3       m_Target;
    BaseGameEntity* m_pTargetAgent;
    PatrolPath*     m_pPath;

    //the current position on the wander circle the agent is
    //attempting to steer towards
    glm::vec3 m_WanderTarget;

    //explained above
    float m_WanderJitter;
    float m_WanderRadius;
    float m_WanderDistance;
    //multipliers. These can be adjusted to effect strength of the
    //appropriate behavior. Useful to get flocking the way you require
    //for example.
    float m_WeightWander;
    float m_WeightSeek;
    float m_WeightFlee;
    float m_WeightArrive;
    float m_WeightFollowPath;
    float m_WeightFollowWaypoints;

    //binary flags to indicate whether or not a behavior should be active
    int m_Flags;

    //Arrive makes use of these to determine how quickly a vehicle
    //should decelerate to its target
    enum Deceleration {
        slow   = 1,
        normal = 2,
        fast   = 3
    };

    //default
    Deceleration m_Deceleration;

    //what type of method is used to sum any active behavior
    summing_method m_SummingMethod;

    //this function tests if a specific bit of m_Flags is set
    bool On(behavior_type behaviorType) {
        return (m_Flags & behaviorType) == behaviorType;
    }

    /* .......................................................

                    BEGIN BEHAVIOR DECLARATIONS

      .......................................................*/

    //this behavior moves the agent towards a target position
    glm::vec3 Seek(glm::vec3 targetPos);
    glm::vec3 Flee(glm::vec3 targetPos);
    glm::vec3 Arrive(glm::vec3 targetPos, Deceleration deceleration);

    glm::vec3 Wander();
    glm::vec3 FollowPath(PatrolPath* path);
    glm::vec3 FollowWaypoints(PatrolPath* path);
    glm::vec3 SeekPathStart(PatrolPath* path);

    //calculates and sums the steering forces from any active behaviors
    glm::vec3 CalculateWeightedSum();
};

#endif // STEERING_BEHAVIOUR_H
