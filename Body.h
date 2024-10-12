//
// Created by me on 8/12/24.
//

#ifndef BODY_H
#define BODY_H

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Shape.h"

class Body {
  public:
	[[nodiscard]] glm::vec3 GetCenterOfMassWorldSpace() const;
	[[nodiscard]] glm::vec3 GetCenterOfMassModelSpace() const;

	glm::vec3 WorldSpaceToBodySpace(glm::vec3 &pt) const;
	glm::vec3 BodySpaceToWorldSpace(glm::vec3 &pt) const;

	void ApplyImpulseLinear(glm::vec3 &impulse);

	glm::vec3 m_Position;
	glm::quat m_Orientation;
	glm::vec3 m_LinearVelocity;
	float m_InvMass;
	float m_Elasticity;
	Shape *m_Shape;
};

#endif // BODY_H
