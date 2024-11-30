#ifndef PATROL_PATH_H
#define PATROL_PATH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <assert.h>

#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"

class PatrolPath; // Forward declare so that Waypoint struct can use it.

struct Waypoint {
    glm::vec3   position;
    std::string targetname;
    std::string target;
    PatrolPath* pPatrolPath = nullptr; // nullptr = doesn't belong to a patrol path
};

class PatrolPath {
public:
    PatrolPath()
        : m_Points(),
          m_Radius(25.0),
          m_direction(1),
          m_CurrentWaypointName(""),
          m_NextWaypointName(""),
          m_PreviousWaypointName("")
  {};

    PatrolPath(std::string pathname)
        : m_Points(),
          m_Name(pathname),
          m_Radius(25.0),
          m_direction(1),
          m_CurrentWaypointName(""),
          m_NextWaypointName(""),
          m_PreviousWaypointName("")
  {};

    PatrolPath(const PatrolPath* other) {
        assert( other != nullptr );
        for (int i = 0; i < other->m_Points.size(); i++) {
            Waypoint point = other->m_Points[i];
            // Makes sure that the point's path pointer points to this.
            // Also sets the m_CurrentWaypointName on first call.
            AddPoint(point); 
        }
        m_Name = other->m_Name;
        m_Radius = other->m_Radius;
        m_direction = other->m_direction;
        m_NextWaypointName = other->m_NextWaypointName;
        m_PreviousWaypointName = "";
    }

    void SetName(std::string name) {
        m_Name = name;
    }

    void                  AddPoint(Waypoint point);
    std::vector<Waypoint> GetPoints();
    float                 GetRadius();
    bool                  IsCurrentWaypointReached(glm::vec3 position);
    void                  TargetNextWaypoint();
    Waypoint              GetCurrentWaypoint();
    void                  SetCurrentWaypoint(std::string targetname);
    void                  SetNextWaypoint(std::string targetname);
    std::vector<Vertex>   GetPointsAsVertices();
  
public:
    int m_direction; // 1 is forward and -1 is backward
  
private:
    std::vector<Waypoint>                     m_Points;
    std::unordered_map<std::string, Waypoint> m_TargetnameToWaypoint;
    float                                     m_Radius;
    std::string                               m_Name;
    bool                                      m_IsClosed;
    std::string                               m_CurrentWaypointName;
    std::string                               m_NextWaypointName;
    std::string                               m_PreviousWaypointName;
};

#endif
