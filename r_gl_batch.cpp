#include "r_gl_batch.h"

#include <glad/glad.h>

#include <string>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"

#include "r_common.h"
#include "r_gl_texture.h"

//struct Vertex {
//    glm::vec3 pos;
//    glm::vec2 uv;
//    glm::vec3 bc;
//    glm::vec3 normal;
//    glm::vec4 color;
//    uint32_t  blendindices[4];
//    glm::vec4 blendweights;
//};

GLBatch::GLBatch(uint32_t maxVerts)
{
    m_GeometryType = GEOM_TYPE_VERTEX_ONLY;

    m_MaxVerts = maxVerts;

    m_MaxIndices = 0;

    m_NumVerts        = 0;
    m_VertOffsetIndex = 0;

    m_IndexOffsetIndex = 0;

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // FIX: Make configurable if data is dynamic or static.
    glBufferData(GL_ARRAY_BUFFER, m_MaxVerts * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // Input assembly for vertex shader

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_POS_OFFSET);

    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_UV_OFFSET);

    glEnableVertexAttribArray(2); // uv lightmap
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_UV_LIGHTMAP_OFFSET);

    glEnableVertexAttribArray(3); // bc
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_BC_OFFSET);

    glEnableVertexAttribArray(4); // normal
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_NORMAL_OFFSET);

    glEnableVertexAttribArray(5); // color
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_COLOR_OFFSET);

    glEnableVertexAttribArray(6); // blendindices
    glVertexAttribIPointer(6, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)VERT_BLENDINDICES_OFFSET);

    glEnableVertexAttribArray(7); // blendweights
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_BLENDWEIGHTS_OFFSET);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

GLBatch::GLBatch(uint32_t maxVerts, uint32_t maxIndices)
{
    m_GeometryType = GEOM_TYPE_INDEXED;

    m_MaxVerts   = maxVerts;
    m_MaxIndices = maxIndices;

    m_NumVerts        = 0;
    m_VertOffsetIndex = 0;

    m_NumIndices       = 0;
    m_IndexOffsetIndex = 0;
    m_LastIndex        = 0;

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_MaxVerts * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // Input assembly for vertex shader

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_POS_OFFSET);

    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_UV_OFFSET);

    glEnableVertexAttribArray(2); // uv lightmap
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_UV_LIGHTMAP_OFFSET);

    glEnableVertexAttribArray(3); // bc
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_BC_OFFSET);

    glEnableVertexAttribArray(4); // normal
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_NORMAL_OFFSET);

    glEnableVertexAttribArray(5); // color
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_COLOR_OFFSET);

    glEnableVertexAttribArray(6); // blendindices
    glVertexAttribIPointer(6, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)VERT_BLENDINDICES_OFFSET);

    glEnableVertexAttribArray(7); // blendweights
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)VERT_BLENDWEIGHTS_OFFSET);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Index buffer

    glGenBuffers(1, &m_iVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MaxIndices * sizeof(uint16_t), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

// TODO: cullFace and drawMode not used!
int GLBatch::AddTris(Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode)
{
    if ( m_VertOffsetIndex + 3 * numTris > m_MaxVerts )
    {
        printf("No more space on GPU to upload more triangles!\nSpace available: %d\n", m_MaxVerts - m_VertOffsetIndex);
        return -1;
    }

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, m_VertOffsetIndex * sizeof(Vertex), numTris * sizeof(Tri), tris);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int offset = m_VertOffsetIndex;
    m_VertOffsetIndex += 3 * numTris;

    return offset;
}

int GLBatch::AddMapTris(MapTri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode)
{
    if ( m_VertOffsetIndex + 3 * numTris > m_MaxVerts )
    {
        printf("No more space on GPU to upload more triangles!\nSpace available: %d\n", m_MaxVerts - m_VertOffsetIndex);
        return -1;
    }

    std::vector<Tri> rawTris{};
    rawTris.resize(numTris);
    for ( int i = 0; i < numTris; i++ )
    {
        rawTris[ i ] = tris[ i ].tri;
    }

    return AddTris(rawTris.data(), numTris, cullFace, drawMode);
}

int GLBatch::AddVertices(const Vertex* verts, uint32_t numVerts, bool cullFace, DrawMode drawMode)
{
    if ( m_VertOffsetIndex + numVerts > m_MaxVerts )
    {
        printf("No more space on GPU to upload more vertices!\nSpace available: %d\n", m_MaxVerts - m_VertOffsetIndex);
        return -1;
    }

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, m_VertOffsetIndex * sizeof(Vertex), numVerts * sizeof(Vertex), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int offset = m_VertOffsetIndex;
    m_VertOffsetIndex += numVerts;

    return offset;
}

bool GLBatch::AddIndexedVertices(Vertex*   verts,
                  uint32_t  numVerts,
                  uint16_t* indices,
                  uint32_t  numIndices,
                  int*      out_offset,
                  int*      out_idxOffset,
                  bool      cullFace,
                  DrawMode  drawMode)
{
    if ( m_VertOffsetIndex + numVerts > m_MaxVerts )
    {
        printf("No more space on GPU to upload more vertices!\nSpace available: %d\n", m_MaxVerts - m_VertOffsetIndex);
        return false;
    }

    if ( m_IndexOffsetIndex + numIndices > m_MaxIndices )
    {
        printf("No more space on GPU to upload more indices!\nSpace available: %d\n",
               m_MaxIndices - m_IndexOffsetIndex);
        return false;
    }

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, m_VertOffsetIndex * sizeof(Vertex), numVerts * sizeof(Vertex), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    *out_offset = m_VertOffsetIndex;
    m_VertOffsetIndex += numVerts;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iVBO);
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER, m_IndexOffsetIndex * sizeof(uint16_t), numIndices * sizeof(uint16_t), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    *out_idxOffset = m_IndexOffsetIndex;
    m_IndexOffsetIndex += numIndices;

    glBindVertexArray(0);

    return true;
}

void GLBatch::Bind()
{
    glBindVertexArray(m_VAO);

    // WE HAVE TO BIND THIS EXPLICITLY... WHY??? OpenGL is nuts.
    if ( m_GeometryType == GEOM_TYPE_INDEXED )
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iVBO);
    }
}

void GLBatch::Unbind()
{
    glBindVertexArray(0);
}

void GLBatch::Reset()
{
    m_NumVerts         = 0;
    m_VertOffsetIndex  = 0;
    m_NumIndices       = 0;
    m_IndexOffsetIndex = 0;
    m_LastIndex        = 0;
}

void GLBatch::Kill()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

uint32_t GLBatch::VertCount()
{
    return m_VertOffsetIndex;
}

uint32_t GLBatch::IndexCount()
{
    return m_IndexOffsetIndex;
}
