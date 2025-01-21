//
// Created by me on 10/21/24.
//

#include "g_door.h"

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

#include "../../Audio/Audio.h"
#include "../../globals.h"
#include "../../hkd_interface.h"
#include "../../map_parser.h"
#include "../../polysoup.h"
#include "../../r_common.h"
#include "../../r_model.h"
#include "g_door_states.h"

void Door::PostCollisionUpdate() {
    m_pStateMachine->Update();
}

Door::Door(const std::vector<Property>& properties, const std::vector<Brush>& brushes)
    : BaseGameEntity(ET_DOOR),
      m_pStateMachine(nullptr) {

    m_pStateMachine = new StateMachine(this);
    m_pStateMachine->SetCurrentState(DoorClosed::Instance());

    // Get all the door's properties from the MAP file.
    BaseGameEntity::GetProperty<double>(properties, "speed", &m_Speed);
    BaseGameEntity::GetProperty<int>(properties, "angle", &m_Angle);
    BaseGameEntity::GetProperty<int>(properties, "lip", &m_Lip);

    // Load the model from brushes
    m_Model           = CreateModelFromBrushes(brushes);
    IRender* renderer = GetRenderer();
    renderer->RegisterBrush(&m_Model);

    // Get the brush (geometry) that defines this door

    std::vector<MapPolygon> mapPolys{};
    for ( int i = 0; i < brushes.size(); i++ ) {
        std::vector<MapPolygon> polys = createPolysoup(brushes[ i ]);
        std::copy(polys.begin(), polys.end(), std::back_inserter(mapPolys));
    }
    std::vector<MapPolygon> mapTris = triangulate(mapPolys);

    // TODO: This stuff happens quite common. Also: Maybe tris are sufficient?
    glm::vec4 triColor = glm::vec4(1.0f, 0.9f, 0.0f, 1.0f);
    glm::vec3 mins     = glm::vec3(99999.0f);
    glm::vec3 maxs     = glm::vec3(-99999.0f);
    for ( int i = 0; i < mapTris.size(); i++ ) {

        MapPolygon mapPoly = mapTris[ i ];

        Vertex A = { glm::vec3(mapPoly.vertices[ 0 ].pos.x, mapPoly.vertices[ 0 ].pos.y, mapPoly.vertices[ 0 ].pos.z) };
        Vertex B = { glm::vec3(mapPoly.vertices[ 1 ].pos.x, mapPoly.vertices[ 1 ].pos.y, mapPoly.vertices[ 1 ].pos.z) };
        Vertex C = { glm::vec3(mapPoly.vertices[ 2 ].pos.x, mapPoly.vertices[ 2 ].pos.y, mapPoly.vertices[ 2 ].pos.z) };

        float minX = glm::min(A.pos.x, B.pos.x, C.pos.x);
        float minY = glm::min(A.pos.y, B.pos.y, C.pos.y);
        float minZ = glm::min(A.pos.z, B.pos.z, C.pos.z);
        float maxX = glm::max(A.pos.x, B.pos.x, C.pos.x);
        float maxY = glm::max(A.pos.y, B.pos.y, C.pos.y);
        float maxZ = glm::max(A.pos.z, B.pos.z, C.pos.z);

        mins = glm::min(mins, glm::vec3(minX, minY, minZ));
        maxs = glm::max(maxs, glm::vec3(maxX, maxY, maxZ));

        A.color    = triColor;
        B.color    = triColor;
        C.color    = triColor;
        MapTri tri = { A, B, C };

        m_MapTris.push_back(tri);
    }

    m_Mins = mins;
    m_Maxs = maxs;

    // Distance to travel is the width of the door in the travel direction.
    glm::quat qDir;
    if ( m_Angle == -1 ) { // Door moves upward
        qDir = glm::angleAxis(glm::radians(-90.0f), DOD_WORLD_FORWARD);
    } else if ( m_Angle == -2 ) { // Door moves downward
        qDir = glm::angleAxis(glm::radians(90.0f), DOD_WORLD_FORWARD);
    } else {
        qDir = glm::angleAxis(glm::radians((float)m_Angle), DOD_WORLD_UP);
    }

    m_Direction = glm::rotate(qDir, m_Direction);

    // Compute distance to travel
    glm::vec3 minsToMaxs     = maxs - mins;
    glm::vec3 directedLength = m_Direction * minsToMaxs;
    m_Distance               = glm::length(directedLength) - (double)m_Lip;

    // Compute center of audio emitter and create dynamic sound
    // for moving stone against each other.
    // TODO: Sound samples could be loaded through a property which
    // can be set in TrenchBroom.
    glm::vec3 doorCenter = mins + minsToMaxs / 2.0f;
    m_SoundEmitterPos    = doorCenter + directedLength / 2.0f;

    auto sfxLoop  = Audio::LoadSource("sfx/sonniss/Door - Stone Long 02 LOOP.wav", 1.2f, true);
    auto sfxEnd   = Audio::LoadSource("sfx/sonniss/Async_Impact2.wav");
    m_SfxMovement = new DynamicSound(sfxLoop, sfxEnd);
}

bool Door::HandleMessage(const Telegram& telegram) {
    return m_pStateMachine->HandleMessage(telegram);
}
