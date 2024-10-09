//
// Created by me on 8/12/24.
//

#ifndef SHAPE_H
#define SHAPE_H

#include <glm/glm.hpp>

class Shape {
public:
    enum ShapeType {
        SHAPE_SPHERE
    };

    virtual ShapeType GetType() const = 0;
    virtual glm::vec3 GetCenterOfMass() const { return m_CenterOfMass; };

    glm::vec3 m_CenterOfMass;
};





#endif //SHAPE_H
