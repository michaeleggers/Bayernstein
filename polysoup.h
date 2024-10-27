#ifndef _POLYSOUP_H_
#define _POLYSOUP_H_

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "map_parser.h"

static const uint8_t POLY_SOUP_MAGIC[4]   = { 'P', 'L', 'Y', '\0' };
static const uint8_t POLY_SOUP_VERSION[4] = { '0', '0', '1', '\0' };

enum SoupFlags {
    SOUP_GET_ALL,
    SOUP_GET_WORLDSPAWN_ONLY
};

struct QuakeMapVertex {
    glm::f64vec3  pos;
    glm::vec2     uv;
};

struct MapPolygon
{
    std::vector<QuakeMapVertex> vertices;
    glm::f64vec3                normal;
    // NOTE: (Michael): A single, solid color would be good for testing
    // the lightmapper. Thus, define a few solid color textures 
    // to use in TrenchBroom.
    std::string                 textureName;  // located in /game/textures/<*.png>
    uint64_t                    surfaceFlags; // eg. transparency, emissiveness, ...
    uint64_t                    contentFlags; // eg. water, slime, lava, ...
};

struct MapPlane
{
    glm::f64vec3 n;
    glm::f64vec3 p0;
    double d;       // = n dot p0. Just for convenience.
};

struct PolySoupDataHeader {
    uint8_t     magic[4];   // must be PLYS
    uint8_t     version[4];
    uint64_t    numPolys;   // num of MapPolygons
    uint64_t    polySize; // sizeof(MapPolygon)
};

#pragma pack(push,1)
struct PolySoupData {
    PolySoupDataHeader  header;
    MapPolygon*         polys;
};
#pragma pack(pop)

std::string             loadTextFile(std::string file);
void                    writePolys(std::string fileName, std::vector<MapPolygon> polys);
void                    writePolysOBJ(std::string fileName, std::vector<MapPolygon> polys);
void                    writePolySoupBinary(std::string fileName, const std::vector<MapPolygon>& polys);
MapPlane                createPlane(glm::f64vec3 p0, glm::f64vec3 p1, glm::f64vec3 p2);
glm::f64vec3            convertVertexToVec3(MapVertex v);
MapPlane                convertFaceToPlane(Face face);
bool                    intersectThreePlanes(MapPlane p0, MapPlane p1, MapPlane p2, glm::f64vec3* intersectionPoint);
bool                    vec3IsEqual(const glm::f64vec3& lhs, const glm::f64vec3& rhs);
void                    insertVertexToPolygon(glm::f64vec3 v, MapPolygon* p);
bool                    isPointInsideBrush(Brush brush, glm::f64vec3 intersectionPoint);
std::vector<MapPolygon> createPolysoup(Map map, SoupFlags soupFlags = SOUP_GET_WORLDSPAWN_ONLY); 
std::vector<MapPolygon> createPolysoup(const Brush& brush);
bool                    isAngleLegal(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1);
double                  getAngle(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1);
MapPolygon              sortVerticesCCW(MapPolygon poly);
std::vector<MapPolygon> triangulate(std::vector<MapPolygon> polys);


#endif
