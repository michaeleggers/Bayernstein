#include <assert.h>

#include <vector>
#include <unordered_map>

#include "./path.h"
#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"

void PatrolPath::AddPoint(Waypoint point) {
    point.pPatrolPath = this;
    m_Points.push_back(point);
    m_TargetnameToWaypoint.insert({ point.targetname, point });
    // TODO: Not sure if the path name should be the first waypoint added.
    // TODO: m_CurrentWaypointName probably should be whatever is
    // specified for the entity targeting this path waypoint?
    if ( m_CurrentWaypointName.empty() ) {
        m_Name = point.targetname;
        m_CurrentWaypointName = point.targetname;
        m_NextWaypointName = point.target;
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
    //printf("Distance to waypoint: %f\n", distance);
    
    return distance < m_Radius;
}

void PatrolPath::TargetNextWaypoint() {
    Waypoint currentWaypoint = GetCurrentWaypoint();
    m_PreviousWaypointName = m_CurrentWaypointName;
    m_CurrentWaypointName = currentWaypoint.target;
    //printf("Setting waypoint to: %s\n", m_CurrentWaypointName.c_str());
}

Waypoint PatrolPath::GetCurrentWaypoint() {
    const auto& currentWaypointEntry = m_TargetnameToWaypoint.find( m_CurrentWaypointName );
    assert ( currentWaypointEntry != m_TargetnameToWaypoint.end() );

    return currentWaypointEntry->second;
}

void PatrolPath::SetCurrentWaypoint(std::string targetname) {
    m_CurrentWaypointName = targetname;
}

void PatrolPath::SetNextWaypoint(std::string targetname) {
    m_NextWaypointName = targetname;
}