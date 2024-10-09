#include "physics.h"

#include "Body.h"
#include "Intersections.h"

void phys_AddBody(Body* body) {
    g_Bodies.push_back(body);
}

void phys_Update(float dt) {

    // Acceleration due to gravity
    glm::vec3 gravity = glm::vec3(0.0f, 0.0f, -10.0f);
    for (int i = 0; i < g_Bodies.size(); i++) {
        //g_Bodies[i]->m_LinearVelocity += glm::vec3(0.0f, 0.0f, -10.0f) * dt;
        Body* body = g_Bodies[i];
        float mass = 1.0f / body->m_InvMass;
        glm::vec3 gravityForce = mass * gravity;
        glm::vec3 impulse = gravityForce * dt;
        body->ApplyImpulseLinear(impulse);
    }

    // Check for collisions with other bodies
    for (int i = 0; i < g_Bodies.size(); i++) {
        for (int j = i + 1; j < g_Bodies.size(); j++) {
            Body* bodyA = g_Bodies[i];
            Body* bodyB = g_Bodies[j];

            // Bodies with infinite mass don't effect each other.
            if (0.0f == bodyA->m_InvMass && 0.0f == bodyB->m_InvMass) {
                continue;
            }

            Contact contact;
            if (Intersect(bodyA, bodyB, contact)) {
                phys_ResolveContact(contact);
            }
        }
    }

    // Positional update based on velocity
    for (int i = 0; i < g_Bodies.size(); i++) {
        Body* body = g_Bodies[i];
        body->m_Position += body->m_LinearVelocity * dt;
    }
}

void phys_ResolveContact(Contact& contact) {
    Body* bodyA = contact.bodyA;
    Body* bodyB = contact.bodyB;

    float invMassA = bodyA->m_InvMass;
    float invMassB = bodyB->m_InvMass;
    float elasticityA = bodyA->m_Elasticity;
    float elasticityB = bodyB->m_Elasticity;
    float combinedElasticity = elasticityA * elasticityB;

    glm::vec3 normal = contact.normal;
    glm::vec3 vab = bodyA->m_LinearVelocity - bodyB->m_LinearVelocity;
    float impulseJ = -(1.0f + combinedElasticity) * glm::dot(vab, normal) / (invMassA + invMassB);
    glm::vec3 vectorImpulseJ = impulseJ * normal;

    glm::vec3 vectorImpulseJA =  1.0f * vectorImpulseJ;
    glm::vec3 vectorImpulseJB = -1.0f * vectorImpulseJ;
    bodyA->ApplyImpulseLinear(vectorImpulseJA);
    bodyB->ApplyImpulseLinear(vectorImpulseJB);

    float tA = bodyA->m_InvMass / (bodyA->m_InvMass + bodyB->m_InvMass);
    float tB = bodyB->m_InvMass / (bodyA->m_InvMass + bodyB->m_InvMass);

    glm::vec3 separationDistance = contact.ptOnB_WorldSpace - contact.ptOnA_WorldSpace;

    // Don't separate bodies if they are very close together
    // to avoid jittering.
    /*
    if ( glm::length(separationDistance) < 1.0f ) {
        // Only if at least on of them has infinite mass.
        if (bodyA->m_InvMass <= 0.0f || bodyB->m_InvMass <= 0.0f) {
            return;
        }
    }
    */

    bodyA->m_Position += separationDistance * tA;
    bodyB->m_Position -= separationDistance * tB;
}
