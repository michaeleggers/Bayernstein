//
// Created by me on 9/1/24.
//

#include "CWorld.h"

#include <string.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "map_parser.h"
#include "irender.h"
#include "hkd_interface.h"

void CWorld::InitWorld(glm::vec3 gravity) {
    m_Gravity = gravity;
}

void CWorld::InitWorldFromMap(const Map& map) {
    m_Gravity = glm::vec3(0.0f, 0.0f, -0.5f);

    // Get static geometry from map
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    // Convert to tris
    m_MapTris = CWorld::CreateMapTrisFromMapPolys(polysoup);
}

void CWorld::InitStaticGeometry(std::vector<MapTri> tris) {
    // Static Geometry cannot be initialized twice!
    assert( !m_StaticGeometryInitialized );

    m_MapTris = tris;

    m_StaticGeometryInitialized = true;
}

void CWorld::AddDynamicGeometry(std::vector<MapTri> tris) {
    // Static geometry must be initialized before any other geometry!
    assert( m_StaticGeometryInitialized );
}

std::vector<MapTri> CWorld::CreateMapTrisFromMapPolys(const std::vector<MapPolygon>& mapPolys) {
    // Get the renderer to register the textures
    IRender* renderer = GetRenderer();

    std::vector<MapTri> mapTris{};
    std::vector<MapPolygon> tris = triangulate(mapPolys);
    glm::vec4 triColor = glm::vec4(0.1f, 0.8f, 1.0f, 1.0f);
    for ( int i = 0; i < tris.size(); i++ ) {
        MapPolygon mapPoly = tris[ i ];

        Vertex A = { glm::vec3(mapPoly.vertices[ 0 ].pos.x, mapPoly.vertices[ 0 ].pos.y, mapPoly.vertices[ 0 ].pos.z),
                     mapPoly.vertices[ 0 ].uv };
        Vertex B = { glm::vec3(mapPoly.vertices[ 1 ].pos.x, mapPoly.vertices[ 1 ].pos.y, mapPoly.vertices[ 1 ].pos.z),
                     mapPoly.vertices[ 1 ].uv };
        Vertex C = { glm::vec3(mapPoly.vertices[ 2 ].pos.x, mapPoly.vertices[ 2 ].pos.y, mapPoly.vertices[ 2 ].pos.z),
                     mapPoly.vertices[ 2 ].uv };

        A.color = triColor;
        B.color = triColor;
        C.color = triColor;
        MapTri tri = { .tri = { A, B, C } };
        tri.textureName = mapPoly.textureName;
        //FIX: Search through all supported image formats not just PNG.
        tri.hTexture = renderer->RegisterTextureGetHandle(tri.textureName + ".tga");
        mapTris.push_back(tri);
    }

    return mapTris;
}

