//
// Created by me on 10/21/24.
//

#include "g_door.h"

#include "g_door_states.h"
#include "../../map_parser.h"
#include "../../polysoup.h"
#include "../../r_common.h"

#include <SDL.h>

void Door::Update() {

    m_pStateMachine->Update();
}

Door::Door(const int id, 
           std::vector<Property>& properties, 
           std::vector<Brush>& brushes) : 
    BaseGameEntity(id, ET_DOOR),
    m_pStateMachine(nullptr) {

    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(DoorClosed::Instance());

    // Get all the door's properties from the MAP file.
    
    for (int i = 0; i < properties.size(); i++) {
        Property& prop = properties[ i ];
        if (prop.key == "angle") {
            // TODO: Parse the value (prop.value is always a string)
            // and assing to m_Angle;
        }
        else if (prop.key == "delay") {
        }
        // And so on...
    }

    // Get the brush (geometry) that defines this door
    // Assume just one brush for now... // TODO: Could be more brushes!
    std::vector<MapPolygon> mapPolys = createPolysoup( brushes[ 0 ] );
    std::vector<MapPolygon> mapTris = triangulate(mapPolys);
   
    // NOTE: Just make doors golden for now. Obviously we texture them later.
    // TODO: This stuff happens quite common. Also: Maybe tris are sufficient?
    glm::vec4 triColor = glm::vec4(1.0f, 0.9f, 0.0f, 1.0f); 
    for (int i = 0; i < mapTris.size(); i++) {

        MapPolygon mapPoly = mapTris[ i ];
        
        Vertex A = { glm::vec3(mapPoly.vertices[0].pos.x,
                               mapPoly.vertices[0].pos.y,
                               mapPoly.vertices[0].pos.z) };
        Vertex B = { glm::vec3(mapPoly.vertices[1].pos.x, 
                               mapPoly.vertices[1].pos.y,
                               mapPoly.vertices[1].pos.z) };
        Vertex C = { glm::vec3(mapPoly.vertices[2].pos.x, 
                               mapPoly.vertices[2].pos.y, 
                               mapPoly.vertices[2].pos.z) };
        A.color = triColor;
        B.color = triColor;
        C.color = triColor;
        Tri tri = { A, B, C };

        TriPlane triPlane{};
        triPlane.tri = tri;
        triPlane.plane = CreatePlaneFromTri(triPlane.tri);
        triPlane.tri.a.normal = triPlane.plane.normal;
        triPlane.tri.b.normal = triPlane.plane.normal;
        triPlane.tri.c.normal = triPlane.plane.normal;
        
        m_TriPlanes.push_back(triPlane);
    }
}

bool Door::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}

