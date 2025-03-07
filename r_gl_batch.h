#ifndef _R_GL_BATCH_H_
#define _R_GL_BATCH_H_

#include <stdint.h>

#include <vector>

#include <glad/glad.h>

#include <string>

#include "irender.h"
#include "r_common.h"
#include "r_gl_texture.h"

class GLBatch
{
  public:
    GLBatch(uint32_t maxVerts);
    GLBatch(uint32_t maxVerts, uint32_t maxIndices);

    int  AddTris(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID);
    int  AddMapTris(MapTri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID);
    int  AddVertices(const Vertex* verts, uint32_t numVerts, bool cullFace = true, DrawMode drawMode = DRAW_MODE_LINES);
    bool AddIndexedVertices(Vertex*   verts,
             uint32_t  numVerts,
             uint16_t* indices,
             uint32_t  numIndices,
             int*      out_offset,
             int*      out_idxOffset,
             bool      cullFace = true,
             DrawMode  drawMode = DRAW_MODE_SOLID);
    void Bind();
    void Unbind();
    void Reset();
    void Kill();

    uint32_t VertCount();
    uint32_t IndexCount();

    // FIX: This helps to remember the last index into the vertex buffer. Useful for multiple calls to Add().
    //		But maybe there is a better way to do this since all calls to Add have to set this value, which
    //		is easy to forget. It might be an option to track all eg. indexed 2d rendering state
    //		in the render backend. For now, this is ok.
    uint16_t m_LastIndex;

  private:
    GeometryType m_GeometryType;

    GLuint m_VBO, m_VAO;
    GLuint m_iVBO;

    uint32_t m_MaxVerts;
    uint32_t m_NumVerts;
    int      m_VertOffsetIndex;

    uint32_t m_MaxIndices;
    uint32_t m_NumIndices;
    int      m_IndexOffsetIndex;
};

#endif
