#ifndef _RCOMMON_H_
#define _RCOMMON_H_

#include <stdint.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

enum GeometryType {
	GEOM_TYPE_VERTEX_ONLY,
	GEOM_TYPE_INDEXED
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 bc;
	glm::vec3 normal; // = glm::vec3(0.0f, -1.0f, 0.0f); // points *against* the forward direction (because camera is facing to forward by default)
	glm::vec4 color;
	uint32_t  blendindices[4];
	glm::vec4 blendweights;
};

// Vertex Attribute Layout

#define VERT_POS_OFFSET			 0
#define VERT_UV_OFFSET			 (VERT_POS_OFFSET + sizeof(glm::vec3))
#define VERT_BC_OFFSET			 (VERT_UV_OFFSET + sizeof(glm::vec2))
#define VERT_NORMAL_OFFSET		 (VERT_BC_OFFSET + sizeof(glm::vec3))
#define VERT_COLOR_OFFSET		 (VERT_NORMAL_OFFSET + sizeof(glm::vec3))
#define VERT_BLENDINDICES_OFFSET (VERT_COLOR_OFFSET + sizeof(glm::vec4))
#define VERT_BLENDWEIGHTS_OFFSET (VERT_BLENDINDICES_OFFSET + 4*sizeof(uint32_t))

// Global shader settings

#define SHADER_WIREFRAME_ON_MESH (0x00000001 << 0)
#define SHADER_LINEMODE          (0x00000001 << 1)
#define SHADER_ANIMATED			 (0x00000001 << 2)
#define SHADER_IS_TEXTURED		 (0x00000001 << 3)

#define GOLDEN_RATIO			 1.618033988749
#define HKD_PI                   3.14159265359
#define HKD_EPSILON              0.000000001f
#define ELLIPSOID_VERT_COUNT	 32

struct ShaderSettings {
	glm::uvec4 u32bitMasks; // TODO: This is just to make the Shader happy (Wants 16 bytes by default, not only 4).
};

struct Tri {
	union {
		struct { Vertex a; Vertex b; Vertex c; };
		Vertex vertices[3];
	};
};

struct Line {
	union {
		struct { Vertex a; Vertex b; };
		Vertex vertices[2];
	};
};

struct Plane {
	glm::vec3 normal;
	float	  d;
	glm::vec3 p;
};

struct TriPlane {
	Plane plane;
	Tri   tri;
};

struct Quad {
	union {
		struct {
			Tri a; // top right
			Tri b; // bottom left
		};
		Tri		tris[2];
		Vertex  vertices[6];
		struct { // This just makes access to individual vertices easier.
			Vertex tl;
			Vertex tr;
			Vertex br;
			Vertex br2;
			Vertex bl;
			Vertex tl2;
		};
	};
};

// Representation of a Quad but only stores the four cornerpoints, not whole tris.
struct FaceQuad {
	union {
		struct {
			Vertex a;
			Vertex b;
			Vertex c;
			Vertex d;
		};
		Vertex vertices[4];
	};
};

struct Box {
	union {
		struct {
			Quad front;
			Quad right;
			Quad back;
			Quad left;
			Quad top;
			Quad bottom;
		};
		Quad quads[6];
		Tri  tris[12];
	};
};

// As Box but with more than 2 tris per side.
struct NBox {
	std::vector<Tri> tris;
};

struct Ellipsoid {
	glm::vec3	center;
	float		radiusA; // horizontal radius
	float		radiusB; // vertical radius
	Vertex		vertices[ELLIPSOID_VERT_COUNT];
};

struct MeshEllipsoid {
	float radiusA;
	float radiusB;
	std::vector<Tri> tris;
};

void RotateTri(Tri* tri, glm::vec3 axis, float angle);
void TranslateTri(Tri* tri, glm::vec3 t);
void TransformTri(Tri* tri, glm::mat4 modelMatrix);
void SetTriColor(Tri* tri, glm::vec4 color);
void SubdivTri(Tri* tri, Tri out_tris[]);
void SubdivTri(Tri* tri, Tri out_tris[], uint32_t numIterations);
void SubdivIndexedTri(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, Vertex* out_verts, uint16_t* out_indices);
void SubdivIndexedTri(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, Vertex* out_verts, uint16_t* out_indices, uint32_t numIterations);
Quad CreateQuad(glm::vec3 pos = glm::vec3(0, 0, 0), float width = 1.0f, float height = 1.0f, glm::vec4 color = glm::vec4(1, 0, 0, 1));
void RotateQuad(Quad* quad, glm::vec3 axis, float angle);
void TranslateQuad(Quad* quad, glm::vec3 t);
FaceQuad QuadToFace(Quad* quad);
void SetQuadColor(Quad* quad, glm::vec4 color);
Box	 CreateBox(glm::vec3 scale = glm::vec3(1.0f), glm::vec4 color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
Box  CreateBoxFromAABB(glm::vec3 mins, glm::vec3 maxs);
void TranslateBox(Box* box, glm::vec3 t);
void TransformBox(Box* box, glm::mat4 modelMatrix);
Ellipsoid CreateEllipsoidFromAABB(glm::vec3 mins, glm::vec3 maxs);
MeshEllipsoid CreateUnitEllipsoid(uint32_t numSubdivs);
void TransformEllipsoid(Ellipsoid* ellipsoid, glm::mat4 modelMatrix);
NBox CreateNBox(glm::vec3 scale, uint32_t numSubdivs);
Plane CreatePlaneFromTri(Tri tri);

#endif

