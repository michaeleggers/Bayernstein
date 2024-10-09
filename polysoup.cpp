


#include <string>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <stack>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "polysoup.h"

//#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#define PS_FLOAT_EPSILON	(0.0001)


static std::string loadTextFile(std::string file)
{
	std::ifstream iFileStream;
	std::stringstream ss;
	iFileStream.open(file, std::ifstream::in);
	if (iFileStream.fail()) {
		fprintf(stderr, "Unable to open file: %s!\nExiting...", file.c_str());
		exit(-1);
	}
	ss << iFileStream.rdbuf();
	std::string data = ss.str();
	iFileStream.close();

	return data;
}

static void writePolys(std::string fileName, std::vector<MapPolygon> polys)
{
	std::ofstream oFileStream;
	oFileStream.open(fileName, std::ios::binary | std::ios::out);

	uint32_t numPolys = polys.size();
	oFileStream.write((char*)&numPolys, sizeof(uint32_t));

	for (auto p = polys.begin(); p != polys.end(); p++) {
		for (auto v = p->vertices.begin(); v != p->vertices.end(); v++) {
			oFileStream.write((char*) & *v, sizeof(glm::f64vec3));
		}
	}

	oFileStream.close();
}

static void writePolysOBJ(std::string fileName, std::vector<MapPolygon> polys)
{
	std::stringstream faces;
	std::ofstream oFileStream;
	oFileStream.open(fileName, std::ios::out);
	
	oFileStream << "o Quake-map" << std::endl;

	size_t count = 1;
	for (auto p = polys.begin(); p != polys.end(); p++) {
		faces << "f";
		for (auto v = p->vertices.begin(); v != p->vertices.end(); v++) {
			oFileStream << "v " << std::to_string(v->x) << " " << std::to_string(v->y) << " " << std::to_string(v->z) << std::endl;
			faces << " " << count; count++;
		}
		faces << std::endl;
	}

	oFileStream << faces.rdbuf();
	oFileStream.close();
	
}

MapPlane createPlane(glm::f64vec3 p0, glm::f64vec3 p1, glm::f64vec3 p2)
{
	glm::f64vec3 v0 = p2 - p0;
	glm::f64vec3 v1 = p1 - p0;
	glm::f64vec3 n = glm::normalize(glm::cross(v0, v1));
	double d = -glm::dot(n, p0);

	return { n, p0, d };
}

static inline glm::f64vec3 convertVertexToVec3(MapVertex v)
{
	return glm::f64vec3(v.x, v.y, v.z);
}

MapPlane convertFaceToPlane(Face face)
{
	glm::f64vec3 p0 = convertVertexToVec3(face.vertices[0]);
	glm::f64vec3 p1 = convertVertexToVec3(face.vertices[1]);
	glm::f64vec3 p2 = convertVertexToVec3(face.vertices[2]);
	return createPlane(p0, p1, p2);
}

bool intersectThreePlanes(MapPlane p0, MapPlane p1, MapPlane p2, glm::f64vec3* intersectionPoint)
{	
	glm::f64vec3 n0xn1 = glm::cross(p0.n, p1.n);
	double det = glm::dot(n0xn1, p2.n);

	if (fabs(det) < PS_FLOAT_EPSILON) // Early out if planes do not intersect at single point
		return false;

	*intersectionPoint = (
		-p0.d * (glm::cross(p1.n, p2.n))
		-p1.d * (glm::cross(p2.n, p0.n))
		-p2.d * (glm::cross(p0.n, p1.n))
		) / det;

	return true;
}

bool vec3IsEqual(const glm::f64vec3& lhs, const glm::f64vec3& rhs) {
	return ( glm::abs(lhs.x - rhs.x) < PS_FLOAT_EPSILON 
		&& glm::abs(lhs.y - rhs.y) < PS_FLOAT_EPSILON
		&& glm::abs(lhs.z - rhs.z) < PS_FLOAT_EPSILON );
}

void insertVertexToPolygon(glm::f64vec3 v, MapPolygon* p)
{
	auto v0 = p->vertices.begin();
	if (v0 == p->vertices.end()) {
		p->vertices.push_back(v);
		return;
	}

	for (; v0 != p->vertices.end(); v0++) {
		if (vec3IsEqual(v, *v0)) { // TODO: Is this check actually needed??
			return;
		}
	}
	
	p->vertices.push_back(v);
}

bool isPointInsideBrush(Brush brush, glm::f64vec3 intersectionPoint)
{
	for (int i = 0; i < brush.faces.size(); i++) {
		Face face = brush.faces[i];
		MapPlane plane = convertFaceToPlane(face);
		glm::f64vec3 a = glm::normalize(intersectionPoint - plane.p0);
		double dotProd = glm::dot(plane.n, a);
		if (glm::dot(plane.n, intersectionPoint) + plane.d > PS_FLOAT_EPSILON) // FIXME: HOW DO WE GET IT NUMERICALLY GOOD ENOUGH??
			return false;
	}

	return true;
}

std::vector<MapPolygon> createPolysoup(Map map)
{
	std::vector<MapPolygon> polys;
	for (auto e = map.entities.begin(); e != map.entities.end(); e++) {
		for (auto b = e->brushes.begin(); b != e->brushes.end(); b++) {
			int faceCount = b->faces.size();
			for (int i = 0; i <  faceCount; i++) {
				MapPlane p0 = convertFaceToPlane(b->faces[i]);
				MapPolygon poly = {};
				poly.normal = p0.n;
				for (int j = 0; j < faceCount; j++) {
					MapPlane p1 = convertFaceToPlane(b->faces[j]);
					for (int k = 0; k < faceCount; k++) {
						glm::f64vec3 intersectionPoint;
						MapPlane p2 = convertFaceToPlane(b->faces[k]);
						if (i != j != k) {
							if (intersectThreePlanes(p0, p1, p2, &intersectionPoint)) {
								if (isPointInsideBrush(*b, intersectionPoint)) {
									// TODO: Calculate texture coordinates
									insertVertexToPolygon(intersectionPoint, &poly);
								}
							}
						}
					}
				}
				if (poly.vertices.size() > 0)
					polys.push_back(poly);
			}
		}
	}

	return polys;
}

bool isAngleLegal(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1)
{
	MapPlane polyPlane = createPlane(center, v0, v1);
	glm::f64vec3 b = glm::normalize(v1 - center);
	MapPlane p = createPlane(center, v0, center + polyPlane.n);
	if (glm::dot(p.n, b) < -0.00001) {
		return false;
	}

	return true;
}

double getAngle(glm::f64vec3 center, glm::f64vec3 v0, glm::f64vec3 v1)
{
	glm::f64vec3 a = glm::normalize(v0 - center);
	glm::f64vec3 b = glm::normalize(v1 - center);

	return glm::dot(a, b);
}

MapPolygon sortVerticesCCW(MapPolygon poly)
{
	MapPolygon result;
	size_t vertCount = poly.vertices.size();
	
	if (vertCount < 3)
		return poly; // Actually not a valid polygon

	// Center of poly
	glm::f64vec3 center(0.0f);
	for (auto v = poly.vertices.begin(); v != poly.vertices.end(); v++) {
		center += *v;
	}
	center /= vertCount;

	size_t closestVertexID = 0;
	for (size_t i = 0; i < vertCount-1; i++) {
		glm::f64vec3 v0 = poly.vertices[i]; // Find next vertex to v0 with smallest angle

		// Plane definition:
		// glm::f64vec3 v0 = p2 - p0;
		// glm::f64vec3 v1 = p1 - p0;
		// glm::f64vec3 n = glm::normalize(glm::cross(v0, v1));
		MapPlane plane = createPlane(center, v0, center + poly.normal);

		int smallesAngleIndex = 0;
		double smallestAngle = -1.0;
		for (size_t j = i+1; j < vertCount; j++) {
			glm::f64vec3 test = glm::normalize(poly.vertices[j] - center);
			if (glm::dot(plane.n, test) < -PS_FLOAT_EPSILON) { // check if point is legal
				double angle = getAngle(center, v0, poly.vertices[j]);
				if (angle > smallestAngle) {
					smallestAngle = angle;
					smallesAngleIndex = j;
				}
			}
		}	
		std::swap(poly.vertices[i+1], poly.vertices[smallesAngleIndex]);
	}

	// Fix winding
	glm::f64vec3 a = poly.vertices[0] - center;
	glm::f64vec3 b = poly.vertices[1] - center;
	glm::f64vec3 normal = glm::normalize(glm::cross(a, b));
	if (glm::dot(normal, poly.normal) < PS_FLOAT_EPSILON) {
		std::reverse(poly.vertices.begin(), poly.vertices.end());
		poly.normal = normal;
	}

	return poly;
}

/*
* Assumes only convex polygons -> use trivial triangulation approach where
* a triangle-fan gets built, eg:
* 
*       v2______v3                        v2______v3
*       /       |                         /|     /|
*      /        |                        / |    / |
*   v1/         |       -->           v1/  |   /  |
*     \         |                       \  |  /   |
*      \        |                        \ | /    | 
*       \_______|                         \|/_____|
*        v0     v4                         v0     v4     
* 
* v0 is the 'provoking vertex'. It is the anchor-point of the triangle fan.
* Note that a lot of redundant data is generated: (v0, v1, v2), (v0, v2, v3), (v0, v3, v4).
* 
* TODO: Fix this with indexed data.
*/
std::vector<MapPolygon> triangulate(std::vector<MapPolygon> polys)
{
	std::vector<MapPolygon> tris = { };

	for (auto p = polys.begin(); p != polys.end(); p++) {
		MapPolygon sortedPoly = sortVerticesCCW(*p);
		size_t vertCount = sortedPoly.vertices.size();		
		glm::f64vec3 provokingVert = sortedPoly.vertices[0];
		for (size_t i = 2; i < vertCount; i++) {
			MapPolygon poly = { };
			poly.vertices.push_back(provokingVert);
			poly.vertices.push_back(sortedPoly.vertices[i - 1]);
			poly.vertices.push_back(sortedPoly.vertices[i]);
			tris.push_back(poly);
		}		
	}

	return tris;
}
/*
int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "No .map provided! Usage:\npolysoup <mapfile>");
		exit(-1);
	}

	MapVersion mapVersion = QUAKE;

	int arg_ = 1;
	char** argv_ = argv + 1;
	while (arg_ < argc) {
		if (!strcmp("-valve", *argv_)) {
			mapVersion = VALVE_220;
			break;
		}
		argv_++; arg_++;
	}

	std::string mapData = loadTextFile(argv[1]);
	size_t inputLength = mapData.length();
	Map map = getMap(&mapData[0], inputLength, mapVersion);	
	std::vector<MapPolygon> polysoup = createPolysoup(map);
	std::vector<MapPolygon> tris = triangulate(polysoup);
	writePolys("tris.bin", tris);
	writePolysOBJ("tris.obj", tris);

	printf("done!\n");

	return 0;
}
*/
