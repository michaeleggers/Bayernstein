#ifndef _POLYSOUP_H_
#define _POLYSOUP_H_

#include <vector>
#include <string>

#include "map_parser.h"

struct MapPolygon
{
	std::vector<glm::f64vec3>  vertices;
	glm::f64vec3			   normal;
};

struct MapPlane
{
	glm::f64vec3 n;
	glm::f64vec3 p0;
	double d;		// = n dot p0. Just for convenience.
};

std::string 			loadTextFile(std::string file);
void 					writePolys(std::string fileName, std::vector<MapPolygon> polys);
void 					writePolysOBJ(std::string fileName, std::vector<MapPolygon> polys);
MapPlane 				createPlane(glm::f64vec3 p0, glm::f64vec3 p1, glm::f64vec3 p2);
inline glm::f64vec3 	convertVertexToVec3(MapVertex v);
MapPlane 				convertFaceToPlane(Face face);
bool 					intersectThreePlanes(MapPlane p0, MapPlane p1, MapPlane p2, glm::f64vec3* intersectionPoint);
bool 					vec3IsEqual(const glm::f64vec3& lhs, const glm::f64vec3& rhs);
void 					insertVertexToPolygon(glm::f64vec3 v, MapPolygon* p);
bool 					isPointInsideBrush(Brush brush, glm::f64vec3 intersectionPoint);
std::vector<MapPolygon> createPolysoup(Map map);
bool 					isAngleLegal(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1);
double 					getAngle(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1);
MapPolygon 				sortVerticesCCW(MapPolygon poly);
std::vector<MapPolygon> triangulate(std::vector<MapPolygon> polys);


#endif
