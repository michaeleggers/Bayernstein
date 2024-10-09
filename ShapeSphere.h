//
// Created by me on 8/12/24.
//

#ifndef SHAPESPHERE_H
#define SHAPESPHERE_H

#include "Shape.h"

#include <glm/glm.hpp>

class ShapeSphere : public Shape {
public:
    ShapeSphere(float radius) : m_Radius (radius) {
        m_CenterOfMass = glm::vec3(0.0f);
    };
    ShapeType GetType() const override { return SHAPE_SPHERE; };

    float m_Radius;
};



#endif //SHAPESPHERE_H
