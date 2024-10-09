#ifndef _IQM_LOADER_H_
#define _IQM_LOADER_H_

#include <string>
#include <vector>

#include <stdint.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

struct IQMHeader {
    uint8_t  magic[16]; // "INTERQUAKEMODEL\0"
    uint32_t version;  // Must be version 2
    uint32_t filesize;
    uint32_t flags;
    uint32_t numText, ofsText;
    uint32_t numMeshes, ofsMeshes;
    uint32_t numVertArrays, numVertices, ofsVertArrays;
    uint32_t numTris, ofsTris, ofsAdjacency;
    uint32_t numJoints, ofsJoints;
    uint32_t numPoses, ofsPoses;
    uint32_t numAnims, ofsAnims;
    uint32_t numFrames, numFrameChannels, ofsFrames, ofsBounds;
    uint32_t numComment, ofsComment;
    uint32_t numExt, ofsExt;
};

struct IQMMeshData {
    uint32_t name;
    uint32_t material;
    uint32_t firstVert, numVerts;
    uint32_t firstTri, numTris;
};

// Default Vertex format

struct IQMVertex {
    float pos[3];
    float texCoord[2];
    float normal[3];
    float tangent[4];
    uint8_t blendindices[4];
    uint8_t blendweights[4];
    uint8_t color[4];
};

// NOTE: Mesh, Pose and Anim structs are not part of IQM.
//       Maybe move them out of here later or something.

struct IQMMesh {
    std::string             material;
    std::vector<IQMVertex>  vertices;
    uint32_t                firstTri, numTris;
};

struct Pose {
    int32_t     parent;
    glm::vec3   translations;
    glm::vec3   scale;
    glm::quat   rotation;
};

struct Anim {
    std::string name;
    uint32_t    firstFrame, numFrames;
    float       framerate;
};

struct Frame {
    glm::vec3   bbmins;
    glm::vec3   bbmaxs;
    float       xyRadius;
    float       sphericalRadius;
};

struct IQMModel {
    std::string             filename;
    std::vector<IQMMesh>    meshes;
    std::vector<glm::mat4>  bindPoses;
    std::vector<glm::mat4>  invBindPoses;
    std::vector<Pose>       poses;
    uint32_t                numJoints;
    uint32_t                numFrames;
    std::vector<Anim>       animations;
    std::vector<Frame>      frameData; // Only holds bounding boxes
};

struct IQMVertArray {
    uint32_t type;
    uint32_t flags;
    uint32_t format;
    uint32_t size;
    uint32_t offset;
};

enum IQMVertArrayType {
    IQM_POSITION,
    IQM_TEXCOORD,
    IQM_NORMAL,
    IQM_TANGENT,
    IQM_BLENDINDIXES,
    IQM_BLENDWEIGHTS,
    IQM_COLOR,

    IQM_CUSTOM = 0x10
};

enum IQMVertArrayFormat {
    IQM_BYTE,
    IQM_UBYTE,
    IQM_SHORT,
    IQM_USHORT,
    IQM_INT,
    IQM_UINT,
    IQM_HALF,
    IQM_FLOAT,
    IQM_DOUBLE
};

struct IQMTri {
    uint32_t vertex[3];
};

struct IQMJoint {
    uint32_t    name;
    int32_t     parent;
    float       translate[3];
    float       rotate[4];
    float       scale[3];
};

struct IQMPose {
    int32_t     parent;
    uint32_t    channelmask;
    float       channeloffset[10];
    float       channelscale[10];
};

struct IQMAnim {
    uint32_t    name;
    uint32_t    firstFrame, numFrames;
    float       framerate;
    uint32_t    flags;
};

struct IQMBounds {
    float bbmins[3];
    float bbmaxs[3];
    float xyradius; // circular radius in xy-plane
    float radius;   // spherical radius
};


IQMModel LoadIQM(const char* file);
void     UnloadIQM(IQMModel* iqmModel);

#endif
