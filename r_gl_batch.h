#ifndef _R_GL_BATCH_H_
#define _R_GL_BATCH_H_

#include <stdint.h>

#include <vector>

#include <glad/glad.h>

#include <string>

#include "r_common.h"
#include "r_gl_texture.h"
#include "irender.h"


class GLBatch {
public:
	GLBatch(uint32_t maxVerts);
	GLBatch(uint32_t maxVerts, uint32_t maxIndices);

	int				Add(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID);
	int				Add(Vertex* verts, uint32_t numVerts, bool cullFace = true, DrawMode drawMode = DRAW_MODE_LINES);
	bool			Add(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, int* out_offset, int* out_idxOffset, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID);
	void			Bind();
	void			Reset();
	void			Kill();

	uint32_t		VertCount();
	
private:
	GeometryType m_GeometryType;

	GLuint		m_VBO, m_VAO;
	GLuint      m_iVBO;

	uint32_t	m_MaxVerts;
	uint32_t	m_NumVerts;
	int			m_VertOffsetIndex;	

	uint32_t	m_MaxIndices;
	uint32_t	m_NumIndices;
	int			m_IndexOffsetIndex;
};

#endif
