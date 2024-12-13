#include "steering_behaviour.h"
#include "../utils/quick_math.h"
#include "../utils/utils.h"
#include <stdio.h>

//------------------------- ctor -----------------------------------------
//
//------------------------------------------------------------------------
SteeringBehaviour::SteeringBehaviour(MovingEntity* pEntity)
    : m_pEntity(pEntity),
      m_Flags(0),
      m_WeightWander(1.0f),
      m_WeightSeek(1.0f),
      m_WeightFlee(1.0f),
      m_WeightArrive(1.0f),
      m_WeightFollowPath(1.0f),
      m_WeightFollowWaypoints(1.5f),
      m_Deceleration(normal),
      m_WanderDistance(16.0f),
      m_WanderJitter(0.0f),
      m_WanderRadius(11.5f),
      m_SteeringForce(0.0f),
      m_Target(0.0f),
      m_SummingMethod(weighted_average),
      m_pTargetAgent(nullptr),
      m_pPath(nullptr),
      m_WanderTarget(0.0)

{
}

glm::vec3 SteeringBehaviour::CalculateWeightedSum() {
    if ( On(seek) && m_pTargetAgent ) {
        m_SteeringForce += Seek(m_pTargetAgent->m_Position) * m_WeightSeek;
    }
    if ( On(flee) && m_pTargetAgent ) {
        m_SteeringForce += Flee(m_pTargetAgent->m_Position) * m_WeightFlee;
    }
    if ( On(arrive) && m_pTargetAgent ) {
        m_SteeringForce += Arrive(m_pTargetAgent->m_Position, m_Deceleration) * m_WeightArrive;
    }
    if ( On(wander) ) {
        m_SteeringForce += Wander() * m_WeightWander;
    }

    if ( On(follow_waypoints) ) {
        if ( m_pPath != nullptr ) {
        m_SteeringForce += FollowWaypoints(m_pPath) * m_WeightFollowWaypoints;
        } else {
            printf("SteeringBehaviour::FollowWaypoints: m_pPath is nullptr! Ignore.\n");
        }
    }
    if ( On(follow_path) ) {
        if ( m_pPath != nullptr ) {
        m_SteeringForce += FollowPath(m_pPath) * m_WeightFollowPath;
        } else {
            printf("SteeringBehaviour::FollowPath: m_pPath is nullptr! Ignore.\n");
        }
    }

    glm::vec3 truncatedForce = math::TruncateVec3(m_SteeringForce, m_pEntity->m_MaxForce);
    truncatedForce           = truncatedForce * 0.5f;
    return truncatedForce;
}

//------------------------------- Wander ---------------------------------
//
// a smoother random walk. by optaining a random target on the unit
// circle and moving this point infront of the agent we get a smooth
// random walk.
//
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Wander() {
    glm::vec3 circlePosition = m_pEntity->m_Position + m_pEntity->m_Forward * m_WanderDistance;
    m_WanderJitter += RandBetween(-3, 3) * (float)GetDeltaTime() / 100.0f;

    glm::vec3 circleOffset
        = glm::vec3(m_WanderRadius * glm::cos(m_WanderJitter), m_WanderRadius * glm::sin(m_WanderJitter), 0.0);
    m_WanderTarget = circlePosition + circleOffset;

    glm::vec3 force = Seek(m_WanderTarget);
    force.z         = 0.0f;
    return force;
}

//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Seek(glm::vec3 targetPosition) {
    glm::vec3 desiredVelocity = glm::normalize(targetPosition - m_pEntity->m_Position) * m_pEntity->m_MaxSpeed;

    return (desiredVelocity - m_pEntity->m_Velocity);
}

//----------------------------- Flee -------------------------------------
//
//  Does the opposite of Seek
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Flee(glm::vec3 targetPosition) {
    //only flee if the target is within 'panic distance'. Work in distance
    //squared space.
    /* const double PanicDistanceSq = 100.0f * 100.0;
  if (Vec2DDistanceSq(m_pEntity->GetPosition(), target) > PanicDistanceSq)
  {
    return glm::vec3(0,0);
  }
  */

    glm::vec3 desiredVelocity = glm::normalize(m_pEntity->m_Position - targetPosition) * m_pEntity->m_MaxSpeed;

    return (desiredVelocity - m_pEntity->m_Velocity);
}

//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Arrive(glm::vec3 targetPosition, Deceleration deceleration) {
    glm::vec3 toTarget = targetPosition - m_pEntity->m_Position;

    //calculate the distance to the target
    double dist = glm::length(toTarget);

    if ( dist > 0 ) {
        //because Deceleration is enumerated as an int, this value is required
        //to provide fine tweaking of the deceleration..
        const double decelerationTweaker = 1000.0f;

        //calculate the speed required to reach the target given the desired deceleration
        double speed = dist / ((double)deceleration * decelerationTweaker);

        //make sure the velocity does not exceed the max
        speed = glm::min(speed, (double)m_pEntity->m_MaxSpeed);

        //from here proceed just like Seek except we don't need to normalize
        //the toTarget vector because we have already gone to the trouble
        //of calculating its length: dist.
        glm::vec3 desiredVelocity = toTarget * (float)speed / (float)dist;

        return (desiredVelocity - m_pEntity->m_Velocity);
    }

    return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 SteeringBehaviour::FollowPath(PatrolPath* path) {
    if ( path->IsEndOfSegmentReached(m_pEntity->m_Position) ) {
        printf("\n\n current waypoint reached\n\n");
        path->TargetNextWaypoint();
    }
    glm::vec3 futurePosition = m_pEntity->m_Position;

    glm::vec3 target = glm::vec3(0.0f);
    // should actually be the previous Waypoint to current waypoint since we are looking at a segment of the path
    glm::vec3 segmentStart = path->GetCurrentWaypoint().position;
    glm::vec3 segmentEnd   = path->GetNextWaypoint().position;

        // project the future position back onto the line to find a potential target
    glm::vec3 normalPoint = math::GetProjectedPoint(futurePosition, segmentStart, segmentEnd);
    // check if the projected point is actually on the line segment, otherwise use the segment end for now.
    if ( !math::InSegmentRange(normalPoint, segmentStart, segmentEnd) ) {
            normalPoint = segmentEnd;
        }
        // calculate the distance of the future position to the projected point to find the closest segment of the path
        float distanceFromPath = glm::distance(futurePosition, normalPoint);
            if ( path->GetRadius() <= distanceFromPath ) {
        // the target is the normal point on the path plus a little offset in the direction of the path
                target = normalPoint + glm::normalize(normalPoint - segmentStart) * 12.5f;
            } else {
                // NOTE: i don't know why this needs to be done ???
                // if we are inside the radius of the path, we are on the path and need to target the next point
        // just keeping the current velocity would probably be better
                target = segmentEnd;
    }

    glm::vec3 force = Seek(target);
    force.z         = 0.0f;
    return force;
}

glm::vec3 SteeringBehaviour::FollowWaypoints(PatrolPath* pPath) {
    if ( pPath->IsCurrentWaypointReached(m_pEntity->m_Position) ) {
        printf("current waypoint reached\n");
        pPath->TargetNextWaypoint();
    }

    glm::vec3 target = pPath->GetCurrentWaypoint().position;
    glm::vec3 force  = Seek(target);
    force.z          = 0.0f;
    return force;
}

glm::vec3 SteeringBehaviour::Calculate() {
    m_SteeringForce = glm::vec3(0.0f);
    switch ( m_SummingMethod ) {
    case weighted_average:

        m_SteeringForce = CalculateWeightedSum();
        return m_SteeringForce;

        //   case prioritized:

        //     m_SteeringForce = CalculatePrioritized(); break;

        //   case dithered:

        //     m_SteeringForce = CalculateDithered();break;

    default:
        return m_SteeringForce;
    }
}
