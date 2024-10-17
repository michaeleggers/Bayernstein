#ifndef _RGL_H_
#define _RGL_H_

#include "irender.h"

#include <vector>

#include <SDL.h>
#include <glad/glad.h>

#include "r_common.h"
#include "r_model.h"
#include "r_gl_batch.h"
#include "r_gl_shader.h"
#include "r_gl_texture_mgr.h"
#include "r_gl_texture.h"
#include "r_gl_fbo.h"
#include "camera.h"

struct GLMesh {
	int			triOffset, triCount; // Offsets into VBO of tris
	GLTexture*	texture;
};

// Models for entities (Players, Monsters, Pickup Items...)
struct GLModel {
	std::vector<GLMesh> meshes;
};

class GLRender : public IRender {
public:
	virtual bool Init(void)								override;
	virtual void Shutdown(void)							override;
	virtual int  RegisterModel(HKD_Model* model)		override;
	virtual void RegisterFont(CFont* font) override;
	virtual void SetActiveCamera(Camera* camera) override;
	virtual std::vector<ITexture*> ModelTextures(int gpuModelHandle)	override;
	virtual std::vector<ITexture*> Textures(void)       override;
	virtual void ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) override;
	virtual void ImDrawTriPlanes(TriPlane* triPlanes, uint32_t numTriPlanes, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) override;
	virtual void ImDrawIndexed(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) override;
	virtual void ImDrawVerts(Vertex* verts, uint32_t numVerts) override;
	virtual void ImDrawLines(Vertex* verts, uint32_t numVerts, bool close = false) override;
	virtual void ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) override;
	virtual void RenderBegin(void)						override;
	virtual void Render(Camera* camera, HKD_Model** models, uint32_t numModels) override;
	virtual void RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels) override;
	virtual void Begin3D() override;
	virtual void End3D()   override;
	virtual void Begin2D() override;
	virtual void End2D() override;
	virtual void SetFont(CFont* font, glm::vec4 color = glm::vec4(1.0f)) override;
	virtual void DrawText(const std::string& text,
				float x, float y, 
				ScreenSpaceCoordMode coordMode = COORD_MODE_REL) override;
	virtual void DrawBox(float x, float y, float width, float height,
					  ScreenSpaceCoordMode coordMode = COORD_MODE_REL) override;
	virtual void RenderEnd(void)						override;
	virtual void SetWindowTitle(char* windowTitle) override;

	void Flush2D();
	void ExecuteDrawCmds(std::vector<GLBatchDrawCmd>& drawCmds, GeometryType geomType);
	void InitShaders();
	void RegisterColliderModels();
	GLBatchDrawCmd AddLineToBatch(GLBatch* btach, Vertex* verts, uint32_t numVerts, bool close);
	GLBatchDrawCmd AddTrisToBatch(GLBatch* batch, Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode);

private:	
	SDL_Window*					m_Window;
	SDL_GLContext				m_SDL_GL_Conext;

	GLTextureManager*			m_TextureManager;

	Camera*						m_ActiveCamera;

	GLBatch*					m_ModelBatch;
	std::vector<GLBatchDrawCmd> m_ModelDrawCmds;
	
	GLBatch*					m_ImPrimitiveBatch;	
	std::vector<GLBatchDrawCmd> m_PrimitiveDrawCmds;

	GLBatch*					m_ImPrimitiveBatchIndexed;
	std::vector<GLBatchDrawCmd>	m_PrimitiveIndexdDrawCmds;

	GLBatch*	m_Screenspace2dBatch;
	
	Shader*						m_ModelShader;
	std::vector<GLModel>		m_Models;

	Shader*						m_ImPrimitivesShader;

	Shader*						m_ColliderShader;
	GLBatch*					m_ColliderBatch;

	Shader*				m_CompositeShader;
	Shader*		                m_Screenspace2dShader;
	// Offsets into collider batch
	GLBatchDrawCmd				m_EllipsoidColliderDrawCmd;

	CglFBO*		                m_2dFBO;
	CglFBO*				m_3dFBO;
	int							m_WindowWidth;
	int							m_WindowHeight;

	// 2D Rendering state
	CFont*		m_CurrentFont;


};

#endif
