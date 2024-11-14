#include <assert.h>

#include <vector>
#include <unordered_map>

#include "./path.h"
#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"

void PatrolPath::AddPoint(Waypoint point) {
    m_Points.push_back(point);
    m_TargetnameToWaypoint.insert({ point.sTargetname, point });
    if ( m_CurrentWaypointName.empty() ) {
        m_CurrentWaypointName = point.sTargetname;
    }
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
    const auto& currentWaypointEntry = m_TargetnameToWaypoint.find( m_CurrentWaypointName );
    if ( currentWaypointEntry == m_TargetnameToWaypoint.end() ) {
        return false;
    }
    const Waypoint& waypoint = currentWaypointEntry->second;
    glm::vec3 currentWaypointPosition = waypoint.position;
    float distance = glm::distance(position, currentWaypointPosition);
    printf("Distance to waypoint: %f\n", distance);
    
    return distance < m_Radius;
}

void PatrolPath::TargetNextWaypoint() {
    Waypoint currentWaypoint = GetCurrentWaypoint();
    m_PreviousWaypointName = m_CurrentWaypointName;
    m_CurrentWaypointName = currentWaypoint.sTarget;
    printf("Setting waypoint to: %s\n", m_CurrentWaypointName.c_str());
}

Waypoint PatrolPath::GetCurrentWaypoint() {
    const auto& currentWaypointEntry = m_TargetnameToWaypoint.find( m_CurrentWaypointName );
    assert ( currentWaypointEntry != m_TargetnameToWaypoint.end() );

    return currentWaypointEntry->second;
}

