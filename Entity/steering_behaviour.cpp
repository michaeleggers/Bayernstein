#include "steering_behaviour.h"
#include "../utils/quick_math.h"
#include "../utils/utils.h"

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
      m_Deceleration(normal),
      m_WanderDistance(7.0f),
      m_WanderJitter(1.0f),
      m_WanderRadius(5.0f),
      m_SteeringForce(0.0f),
      m_Target(0.0f),
      m_SummingMethod(weighted_average)

{
    //stuff for the wander behavior
    float theta = RandBetween(-1, 1) * glm::two_pi<float>();

    //create a vector to a target position on the wander circle
    m_WanderTarget = glm::vec3(m_WanderRadius * glm::cos(theta), m_WanderRadius * glm::sin(theta), 0.0f);
}

glm::vec3 SteeringBehaviour::CalculateWeightedSum() {
    if ( On(seek) && m_pTargetAgent ) {
        m_SteeringForce += Seek(m_pTargetAgent->GetPosition()) * m_WeightSeek;
    }
    if ( On(flee) && m_pTargetAgent ) {
        m_SteeringForce += Flee(m_pTargetAgent->GetPosition()) * m_WeightFlee;
    }
    if ( On(arrive) && m_pTargetAgent ) {
        m_SteeringForce += Arrive(m_pTargetAgent->GetPosition(), m_Deceleration) * m_WeightArrive;
    }
    if ( On(wander) ) {
        m_SteeringForce += Wander() * m_WeightWander;
    }
    return math::Truncate(m_SteeringForce, m_pEntity->GetMaxForce());
}

glm::vec3 SteeringBehaviour::Wander() {
    //this behavior is dependent on the update rate, so this line must
    //be included when using time independent framerate.
    float jitterThisTimeSlice = m_WanderJitter * GetDeltaTime();

    //first, add a small random vector to the target's position
    m_WanderTarget += glm::vec3(
        RandBetween(-1.0f, 1.0f) * jitterThisTimeSlice, RandBetween(-1.0f, 1.0f) * jitterThisTimeSlice, 0.0f);

    //reproject this new vector back on to a unit circle
    m_WanderTarget = glm::normalize(m_WanderTarget);

    //increase the length of the vector to the same as the radius
    //of the wander circle
    m_WanderTarget *= m_WanderRadius;

    //move the target into a position WanderDist in front of the agent
    glm::vec3 target = m_WanderTarget + m_pEntity->GetForward() * m_WanderDistance;
    //project the target into world space
    target = math::ChangeOfBasis(target, m_pEntity->GetForward(), m_pEntity->GetSide(), m_pEntity->GetUp());

    glm::vec3 position = m_pEntity->GetPosition();
    position.z = 0.0f; // for the random walk we don't want to change the z position

    //and steer towards it
    return target - position;
}

//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Seek(glm::vec3 targetPosition) {
    glm::vec3 desiredVelocity = glm::normalize(targetPosition - m_pEntity->GetPosition()) * m_pEntity->GetMaxSpeed();

    return (desiredVelocity - m_pEntity->GetVelocity());
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

    glm::vec3 desiredVelocity = glm::normalize(m_pEntity->GetPosition() - targetPosition) * m_pEntity->GetMaxSpeed();

    return (desiredVelocity - m_pEntity->GetVelocity());
}

//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
glm::vec3 SteeringBehaviour::Arrive(glm::vec3 targetPosition, Deceleration deceleration) {
    glm::vec3 toTarget = targetPosition - m_pEntity->GetPosition();

    //calculate the distance to the target
    double dist = glm::length(toTarget);

    if ( dist > 0 ) {
        //because Deceleration is enumerated as an int, this value is required
        //to provide fine tweaking of the deceleration..
        const double decelerationTweaker = 1000.0f;

        //calculate the speed required to reach the target given the desired
        //deceleration
        double speed = dist / ((double)deceleration * decelerationTweaker);

        //make sure the velocity does not exceed the max
        speed = glm::min(speed, (double)m_pEntity->GetMaxSpeed());

        //from here proceed just like Seek except we don't need to normalize
        //the toTarget vector because we have already gone to the trouble
        //of calculating its length: dist.
        glm::vec3 desiredVelocity = toTarget * (float)speed / (float)dist;

        return (desiredVelocity - m_pEntity->GetVelocity());
    }

    return glm::vec3(0.0f, 0.0f, 0.0f);
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
