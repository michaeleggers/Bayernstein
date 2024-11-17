#ifndef PATROL_PATH_H
#define PATROL_PATH_H

#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"
#include <string>
#include <vector>
#include <unordered_map>

class PatrolPath; // Forward declare so that Waypoint struct can use it.

struct Waypoint {
    glm::vec3   position;
    std::string sTargetname;
    std::string sTarget;
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
          m_name(pathname),
          m_Radius(25.0),
          m_direction(1),
          m_CurrentWaypointName(""),
          m_NextWaypointName(""),
          m_PreviousWaypointName("")
  {};

    void SetName(std::string name) {
        m_name = name;
    }

    void                  AddPoint(Waypoint& point);
    std::vector<Waypoint> GetPoints();
    float                 GetRadius();
    bool                  IsCurrentWaypointReached(glm::vec3 position);
    void                  TargetNextWaypoint();
    Waypoint              GetCurrentWaypoint();
    std::vector<Vertex>   GetPointsAsVertices();
  
public:
    int m_direction; // 1 is forward and -1 is backward
  
private:
    std::vector<Waypoint>                     m_Points;
    std::unordered_map<std::string, Waypoint> m_TargetnameToWaypoint;
    float                                     m_Radius;
    std::string                               m_name;
    bool                                      m_IsClosed;
    std::string                               m_CurrentWaypointName;
    std::string                               m_NextWaypointName;
    std::string                               m_PreviousWaypointName;
};

#endif

