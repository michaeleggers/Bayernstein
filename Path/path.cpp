
#include "./path.h"
#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"
#include <vector>

void PatrolPath::AddPoint(Waypoint point) {
    m_Points.push_back(point);
}

std::vector<Waypoint> PatrolPath::GetPoints() {
    return m_Points;
}
float PatrolPath::GetRadius() {
    return m_Radius;
}

std::vector<Vertex> PatrolPath::GetPointsAsVertices() {
    if ( m_Points.size() == 0 ) {
        return {};
    }
    std::vector<Vertex> vertices;
    for ( int i = 0; i < m_Points.size(); i++ ) {
        Vertex v = { m_Points[ i ].position };
        vertices.push_back(v);
    }
    return vertices;
}

bool PatrolPath::IsCurrentWaypointReached(glm::vec3 position) {
    glm::vec3 currentWaypointPosition = m_Points[ m_CurrentWaypointIndex ].position;
    float     distance                = glm::distance(position, currentWaypointPosition);
    //printf("Distance to waypoint: %f\n", distance);
    return distance < m_Radius;
}

void PatrolPath::TargetNextWaypoint() {
    m_PreviousWaypointIndex = m_CurrentWaypointIndex;
    m_CurrentWaypointIndex  = m_NextWaypointIndex;
    m_NextWaypointIndex += m_direction;
    if ( m_NextWaypointIndex >= m_Points.size() ) {
        m_NextWaypointIndex = 0;
    }
    if ( m_NextWaypointIndex < 0 ) {
        m_NextWaypointIndex = m_Points.size() - 1;
    }
}

Waypoint PatrolPath::GetCurrentWaypoint() {
    return m_Points[ m_CurrentWaypointIndex ];
}
