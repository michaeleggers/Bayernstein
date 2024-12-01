#ifndef PATROL_PATH_H
#define PATROL_PATH_H

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"

class PatrolPath; // Forward declare so that Waypoint struct can use it.

struct Waypoint {
    glm::vec3   position;
    std::string targetname;
    std::string target;
    PatrolPath* pPatrolPath = nullptr; // nullptr = doesn't belong to a patrol path
};

enum Direction {
    Forward  = 1,
    Backward = 0
};

class PatrolPath {
  public:
    PatrolPath()
        : m_Points(),
          m_Radius(5.0),
          m_direction(Direction::Forward),
          m_CurrentWaypointName(""),
          m_NextWaypointName(""),
          m_PreviousWaypointName("") {};

    PatrolPath(std::string pathname)
        : m_Points(),
          m_Name(pathname),
          m_Radius(5.0),
          m_direction(Direction::Forward),
          m_CurrentWaypointName(""),
          m_NextWaypointName(""),
          m_PreviousWaypointName("") {};

    PatrolPath(const PatrolPath* other) {
        assert(other != nullptr);
        for ( int i = 0; i < other->m_Points.size(); i++ ) {
            Waypoint point = other->m_Points[ i ];
            // Makes sure that the point's path pointer points to this.
            // Also sets the m_CurrentWaypointName on first call.
            AddPoint(point);
        }
        m_Name                 = other->m_Name;
        m_Radius               = other->m_Radius;
        m_direction            = other->m_direction;
        m_OffsetToEntity       = other->m_OffsetToEntity;
        m_PreviousWaypointName = "";
    }

    void SetName(std::string name) {
        m_Name = name;
    }

    void                  AddPoint(Waypoint point);
    std::vector<Waypoint> GetPoints();
    float                 GetRadius();
    bool                  IsCurrentWaypointReached(glm::vec3 position);
    bool                  IsEndOfSegmentReached(glm::vec3 position);
    void                  TargetNextWaypoint();
    Waypoint              GetCurrentWaypoint();
    Waypoint              GetNextWaypoint();
    void                  SetCurrentWaypoint(std::string targetname);
    void                  SetNextWaypoint(std::string targetname);
    std::vector<Vertex>   GetPointsAsVertices();

  public:
    Direction   m_direction;
    float       m_OffsetToEntity = 0.0;
    float       m_Radius;
    std::string m_Name;
    bool        IsClosed() {
        return m_Points.size() > 1 && m_Points[ 0 ].targetname == m_Points[ m_Points.size() - 1 ].target;
    };

  private:
    std::vector<Waypoint>                     m_Points;
    std::unordered_map<std::string, Waypoint> m_TargetnameToWaypoint;
    std::string                               m_CurrentWaypointName;
    std::string                               m_NextWaypointName;
    std::string                               m_PreviousWaypointName;
};

#endif
