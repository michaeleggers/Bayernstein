#ifndef PATROL_PATH_H
#define PATROL_PATH_H

#include "../dependencies/glm/glm.hpp"
#include "../r_common.h"
#include <string>
#include <vector>

struct Waypoint {
    glm::vec3 position;

    int id;
    int target;
};

class PatrolPath {
  public:
    PatrolPath()
        : m_Points(),
          m_Radius(25.0),
          m_direction(1),
          m_CurrentWaypointIndex(0),
          m_NextWaypointIndex(0),
          m_PreviousWaypointIndex(0) {};

    PatrolPath(std::string pathname)
        : m_Points(),
          m_name(pathname),
          m_Radius(15.0),
          m_direction(1),
          m_CurrentWaypointIndex(0),
          m_NextWaypointIndex(0),
          m_PreviousWaypointIndex(0) {};

    void SetName(std::string name) {
        m_name = name;
    }
    void                  AddPoint(Waypoint point);
    std::vector<Waypoint> GetPoints();

    float    GetRadius();
    bool     IsCurrentWaypointReached(glm::vec3 position);
    void     TargetNextWaypoint();
    Waypoint GetCurrentWaypoint();

    std::vector<Vertex> GetPointsAsVertices();

  public:
    int m_direction; // 1 is forward and -1 is backward

  private:
    std::vector<Waypoint> m_Points;

    float       m_Radius;
    std::string m_name;
    bool        m_IsClosed;
    int         m_CurrentWaypointIndex;
    int         m_NextWaypointIndex;
    int         m_PreviousWaypointIndex;
};

#endif
