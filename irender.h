// This is the interface of the renderer. If you want to build a new backend (for example Apple Metal)
// then you have to implement against this interface.

#ifndef _IRENDER_H_
#define _IRENDER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include <SDL.h>

#include "dependencies/glm/glm.hpp"

#include "CWorld.h"
#include "Console/Console.h"
#include "camera.h"
#include "irender.h"
#include "r_common.h"
#include "r_font.h"
#include "r_itexture.h"
#include "r_itexture_mgr.h"
#include "r_model.h"

enum DrawMode
{
    DRAW_MODE_SOLID,
    DRAW_MODE_WIREFRAME,
    DRAW_MODE_LINES
};

struct GLBatchDrawCmd
{                         // TODO: Rename
    int      offset;      // offset into vertex buffer (VBO)
    int      indexOffset; // offset into index buffer (iVBO)
    uint32_t numVerts;
    uint32_t numIndices;
    bool     cullFace;
    DrawMode drawMode;
};

class IRender
{
  public:
    virtual bool                   Init(void)                                                                       = 0;
    virtual void                   SetResolution(int width, int height)                                             = 0;
    virtual void                   SetDisplayMode(DisplayMode displayMode)                                          = 0;
    virtual void                   Shutdown(void)                                                                   = 0;
    virtual int                    RegisterModel(HKD_Model* model)                                                  = 0;
    virtual int                    RegisterBrush(HKD_Model* model)                                                  = 0;
    virtual void                   RegisterFont(CFont* font)                                                        = 0;
    virtual void                   RegisterWorld(CWorld* world)                                                     = 0;
    virtual bool                   RegisterTextureGetHandle(const std::string& name, uint64_t* out_handle)          = 0;
    virtual void                   SetActiveCamera(Camera* camera)                                                  = 0;
    virtual std::vector<ITexture*> ModelTextures(int gpuModelHandle)                                                = 0;
    virtual std::vector<ITexture*> Textures()                                                                       = 0;
    virtual void ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) = 0;
    virtual void ImDrawTriPlanes(TriPlane* triPlanes,
                                 uint32_t  numTriPlanes,
                                 bool      cullFace = true,
                                 DrawMode  drawMode = DRAW_MODE_SOLID)
        = 0;
    virtual void ImDrawIndexed(Vertex*   verts,
                               uint32_t  numVerts,
                               uint16_t* indices,
                               uint32_t  numIndices,
                               bool      cullFace = true,
                               DrawMode  drawMode = DRAW_MODE_SOLID)
        = 0;
    virtual void ImDrawVerts(Vertex* verts, uint32_t numVerts)                                                  = 0;
    virtual void ImDrawLines(const Vertex* verts, uint32_t numVerts, bool close = false)                        = 0;
    virtual void ImDrawCircle(glm::vec3 center, float radius, glm::vec3 normal)                                 = 0;
    virtual void ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) = 0;
    virtual void RenderBegin(void)                                                                              = 0;
    virtual void Begin3D()                                                                                      = 0;
    virtual void End3D()                                                                                        = 0;
    virtual void DrawWorldTris()                                                                                = 0;
    virtual void Begin2D()                                                                                      = 0;
    virtual void End2D()                                                                                        = 0;
    virtual void DrawSprite(const Sprite*        sprite,
                            const glm::vec2&     pos,
                            const glm::vec2&     scaleXY,
                            ScreenSpaceCoordMode coordMode = COORD_MODE_REL)
        = 0;
    virtual void SetFont(CFont* font, glm::vec4 color = glm::vec4(1.0f)) = 0;
    virtual void SetShapeColor(glm::vec4 color = glm::vec4(1.0f))        = 0;
    virtual void FlushFonts()                                            = 0;
    virtual void FlushShapes()                                           = 0;
    virtual void R_DrawText(const std::string& text, float x, float y, ScreenSpaceCoordMode coordMode = COORD_MODE_REL)
        = 0;
    virtual void DrawBox(float x, float y, float width, float height, ScreenSpaceCoordMode coordMode = COORD_MODE_REL)
        = 0;
    virtual void
    Render(Camera* camera, HKD_Model** models, uint32_t numModels, HKD_Model** brushModels, uint32_t numBrushModels)
        = 0;

    virtual void             RenderFirstPersonView(Camera* camera, HKD_Model* model)                 = 0;
    virtual void             RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels) = 0;
    virtual void             RenderConsole(Console* console, CFont* font)                            = 0;
    virtual void             RenderEnd(void)                                                         = 0;
    virtual void             SetWindowTitle(char* windowTitle)                                       = 0;
    virtual glm::vec2        GetWindowDimensions()                                                   = 0;
    virtual glm::vec2        GetRenderDimensions()                                                   = 0;
    virtual SDL_Window*      GetWindow()                                                             = 0;
    virtual ITextureManager* GetTextureManager()                                                     = 0;

  private:
};

#endif
