// This is the interface of the renderer. If you want to build a new backend (for example Apple Metal)
// then you have to implement against this interface.

#ifndef _IRENDER_H_
#define _IRENDER_H_

#include <vector>
#include <string>

#include "irender.h"
#include "r_itexture.h"
#include "r_model.h"
#include "camera.h"
#include "r_common.h"
#include "r_font.h"
#include "Console/Console.h"

enum DrawMode {
	DRAW_MODE_SOLID,
	DRAW_MODE_WIREFRAME,
	DRAW_MODE_LINES
};

struct GLBatchDrawCmd { // TODO: Rename
	int				offset;		// offset into vertex buffer (VBO)
	int				indexOffset;	// offset into index buffer (iVBO)
	uint32_t		numVerts;
	uint32_t		numIndices;
	bool			cullFace;
	DrawMode		drawMode;
};

class IRender {
public:
	virtual bool Init(void)			= 0;
	virtual void Shutdown(void)		= 0;
	virtual int  RegisterModel(HKD_Model* model)	= 0;
	virtual void RegisterFont(CFont* font) = 0;
	virtual void RegisterWorldTris(std::vector<MapTri>& tris) = 0;
	virtual uint64_t RegisterTextureGetHandle(std::string name) = 0;
	virtual void SetActiveCamera(Camera* camera) = 0;
	virtual std::vector<ITexture*> ModelTextures(int gpuModelHandle) = 0;
	virtual std::vector<ITexture*> Textures() = 0;
	virtual void ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) = 0;
	virtual void ImDrawTriPlanes(TriPlane* triPlanes, uint32_t numTriPlanes, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) = 0;
	virtual void ImDrawIndexed(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) = 0;
	virtual void ImDrawVerts(Vertex* verts, uint32_t numVerts) = 0;
	virtual void ImDrawLines(Vertex* verts, uint32_t numVerts, bool close = false) = 0;
	virtual void ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) = 0;
	virtual void RenderBegin(void) = 0;
	virtual void Begin3D() = 0;
	virtual void End3D() = 0;
	virtual void DrawWorldTris() = 0;
	virtual void Begin2D() = 0;
	virtual void End2D() = 0;
	virtual void SetFont(CFont* font, glm::vec4 color = glm::vec4(1.0f)) = 0; 
	virtual void SetShapeColor(glm::vec4 color = glm::vec4(1.0f)) = 0;
	virtual void FlushFonts() = 0;
	virtual void FlushShapes() = 0;
	virtual void R_DrawText(const std::string& text, float x, float y, ScreenSpaceCoordMode = COORD_MODE_REL) = 0; 
	virtual void DrawBox(float x, float y, float width, float height,
					  ScreenSpaceCoordMode coordMode = COORD_MODE_REL) = 0;
	virtual void Render(Camera* camera, HKD_Model** models, uint32_t numModels) = 0;
	virtual void RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels) = 0;
	virtual void RenderConsole(Console* console, CFont* font) = 0;
	virtual void RenderEnd(void) = 0;
	virtual void SetWindowTitle(char* windowTitle) = 0;

private:

};

#endif
