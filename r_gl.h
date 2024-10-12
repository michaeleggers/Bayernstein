#ifndef _RGL_H_
#define _RGL_H_

#include "irender.h"

#include <vector>

#include <SDL.h>
#include <glad/glad.h>

#include "camera.h"
#include "r_common.h"
#include "r_gl_batch.h"
#include "r_gl_shader.h"
#include "r_gl_texture.h"
#include "r_gl_texture_mgr.h"
#include "r_model.h"

struct GLMesh {
	int triOffset, triCount; // Offsets into VBO of tris
	GLTexture *texture;
};

// Models for entities (Players, Monsters, Pickup Items...)
struct GLModel {
	std::vector<GLMesh> meshes;
};

class GLRender : public IRender {
public:
	bool Init() override;

	void Shutdown() override;

	int RegisterModel(HKD_Model *model) override;

	void SetActiveCamera(Camera *camera) override;

	std::vector<ITexture *> ModelTextures(int gpuModelHandle) override;

	std::vector<ITexture *> Textures() override;

	void ImDrawTris(Tri *tris, uint32_t numTris, bool cullFace = true,
	                DrawMode drawMode = DRAW_MODE_SOLID) override;

	void ImDrawTriPlanes(TriPlane *triPlanes, uint32_t numTriPlanes,
	                     bool cullFace = true,
	                     DrawMode drawMode = DRAW_MODE_SOLID) override;

	void ImDrawIndexed(Vertex *verts, uint32_t numVerts, uint16_t *indices,
	                   uint32_t numIndices, bool cullFace = true,
	                   DrawMode drawMode = DRAW_MODE_SOLID) override;

	void ImDrawVerts(Vertex *verts, uint32_t numVerts) override;

	void ImDrawLines(Vertex *verts, uint32_t numVerts,
	                 bool close = false) override;

	void ImDrawSphere(glm::vec3 pos, float radius,
	                  glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f,
	                                              1.0f)) override;

	void RenderBegin() override;

	void Render(Camera *camera, HKD_Model **models,
	            uint32_t numModels) override;

	void RenderColliders(Camera *camera, HKD_Model **models,
	                     uint32_t numModels) override;

	void RenderEnd() override;

	void SetWindowTitle(char *windowTitle) override;

	static void ExecuteDrawCmds(std::vector<GLBatchDrawCmd> &drawCmds,
	                            GeometryType geomType);

	void InitShaders();

	void RegisterColliderModels();

	static GLBatchDrawCmd AddLineToBatch(GLBatch *batch, Vertex *verts,
	                                     uint32_t numVerts, bool close);

	static GLBatchDrawCmd AddTrisToBatch(GLBatch *batch, Tri *tris,
	                                     uint32_t numTris, bool cullFace,
	                                     DrawMode drawMode);

private:
	SDL_Window *m_Window{};
	SDL_GLContext m_SDL_GL_Conext{};

	GLTextureManager *m_TextureManager{};

	Camera *m_ActiveCamera{};

	GLBatch *m_ModelBatch{};
	std::vector<GLBatchDrawCmd> m_ModelDrawCmds;

	GLBatch *m_ImPrimitiveBatch{};
	std::vector<GLBatchDrawCmd> m_PrimitiveDrawCmds;

	GLBatch *m_ImPrimitiveBatchIndexed{};
	std::vector<GLBatchDrawCmd> m_PrimitiveIndexdDrawCmds;

	Shader *m_ModelShader{};
	std::vector<GLModel> m_Models;

	Shader *m_ImPrimitivesShader{};

	Shader *m_ColliderShader{};
	GLBatch *m_ColliderBatch{};
	// Offsets into collider batch
	GLBatchDrawCmd m_EllipsoidColliderDrawCmd{};

	int m_WindowWidth{};
	int m_WindowHeight{};
};

#endif
