#ifndef _RGL_H_
#define _RGL_H_

#include "irender.h"

#include <stdint.h>
#include <vector>

#include <SDL.h>
#include <glad/glad.h>

#include "Console/Console.h"
#include "camera.h"
#include "r_common.h"
#include "r_gl_batch.h"
#include "r_gl_fbo.h"
#include "r_gl_shader.h"
#include "r_gl_texture.h"
#include "r_gl_texture_mgr.h"
#include "r_itexture_mgr.h"
#include "r_model.h"
#include "utils/quick_math.h"

struct GLMesh
{
    int        triOffset, triCount; // Offsets into VBO of tris
    GLTexture* texture;
};

// Models for entities (Players, Monsters, Pickup Items...)
struct GLModel
{
    std::vector<GLMesh> meshes;
};

class GLRender : public IRender
{
  public:
    virtual bool                   Init(void) override;
    virtual void                   Shutdown(void) override;
    virtual int                    RegisterModel(HKD_Model* model) override;
    virtual int                    RegisterBrush(HKD_Model* model) override;
    virtual void                   RegisterFont(CFont* font) override;
    virtual void                   RegisterWorld(CWorld* world) override;
    virtual uint64_t               RegisterTextureGetHandle(const std::string& name) override;
    virtual void                   SetActiveCamera(Camera* camera) override;
    virtual std::vector<ITexture*> ModelTextures(int gpuModelHandle) override;
    virtual std::vector<ITexture*> Textures(void) override;
    virtual void
    ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace = true, DrawMode drawMode = DRAW_MODE_SOLID) override;
    virtual void ImDrawTriPlanes(TriPlane* triPlanes,
                                 uint32_t  numTriPlanes,
                                 bool      cullFace = true,
                                 DrawMode  drawMode = DRAW_MODE_SOLID) override;
    virtual void ImDrawIndexed(Vertex*   verts,
                               uint32_t  numVerts,
                               uint16_t* indices,
                               uint32_t  numIndices,
                               bool      cullFace = true,
                               DrawMode  drawMode = DRAW_MODE_SOLID) override;
    virtual void ImDrawVerts(Vertex* verts, uint32_t numVerts) override;
    virtual void ImDrawLines(Vertex* verts, uint32_t numVerts, bool close = false) override;
    virtual void ImDrawCircle(glm::vec3 center, float radius, glm::vec3 normal) override;
    virtual void
    ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) override;
    virtual void RenderBegin(void) override;
    virtual void Render(Camera*     camera,
                        HKD_Model** models,
                        uint32_t    numModels,
                        HKD_Model** brushModels,
                        uint32_t    numBrushModels) override;
    virtual void RenderFirstPersonView(Camera* camera, HKD_Model* model) override;
    virtual void RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels) override;
    virtual void RenderConsole(Console* console, CFont* font) override;
    virtual void Begin3D() override;
    virtual void End3D() override;
    virtual void DrawWorldTris() override;
    virtual void Begin2D() override;
    virtual void End2D() override;
    virtual void DrawSprite(const Sprite*        sprite,
                            const glm::vec2&     pos,
                            const glm::vec2&     scale,
                            ScreenSpaceCoordMode coordMode = COORD_MODE_REL) override;
    virtual void SetFont(CFont* font, glm::vec4 color = glm::vec4(1.0f)) override;
    virtual void SetShapeColor(glm::vec4 color = glm::vec4(1.0f)) override;
    virtual void
    R_DrawText(const std::string& text, float x, float y, ScreenSpaceCoordMode coordMode = COORD_MODE_REL) override;
    virtual void FlushFonts() override;
    virtual void FlushShapes() override;
    virtual void
    DrawBox(float x, float y, float width, float height, ScreenSpaceCoordMode coordMode = COORD_MODE_REL) override;
    virtual void        RenderEnd(void) override;
    virtual void        SetWindowTitle(char* windowTitle) override;
    virtual SDL_Window* GetWindow() override
    {
        return m_Window;
    };
    virtual glm::vec2        GetWindowDimensions() override;
    virtual ITextureManager* GetTextureManager() override;

    void DrawFrustum(const math::Frustum& frustum);

    void           ExecuteDrawCmds(std::vector<GLBatchDrawCmd>& drawCmds, GeometryType geomType);
    void           InitShaders();
    void           RegisterColliderModels();
    GLBatchDrawCmd AddLineToBatch(GLBatch* btach, Vertex* verts, uint32_t numVerts, bool close);
    GLBatchDrawCmd AddTrisToBatch(GLBatch* batch, Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode);

  private:
    SDL_Window*   m_Window;
    SDL_GLContext m_SDL_GL_Conext;

    GLTextureManager* m_ITextureManager;

    Camera* m_ActiveCamera;

    // Draw world polygons based on their texture handle.
    std::unordered_map<uint64_t, GLBatchDrawCmd> m_TexHandleToWorldDrawCmd;
    GLBatch*                                     m_WorldBatch;
    GLBatch*                                     m_BrushBatch; // World geometry from brush entities.

    GLBatch*                    m_ModelBatch;
    std::vector<GLBatchDrawCmd> m_ModelDrawCmds;

    GLBatch*                    m_ImPrimitiveBatch;
    std::vector<GLBatchDrawCmd> m_PrimitiveDrawCmds;

    GLBatch*                    m_ImPrimitiveBatchIndexed;
    std::vector<GLBatchDrawCmd> m_PrimitiveIndexdDrawCmds;

    GLBatch* m_FontBatch;
    GLBatch* m_ShapesBatch;

    Shader*              m_ModelShader;
    std::vector<GLModel> m_Models;

    Shader* m_WorldShader;
    Shader* m_BrushShader;

    Shader* m_ImPrimitivesShader;

    Shader*  m_ColliderShader;
    GLBatch* m_ColliderBatch;

    Shader* m_CompositeShader;
    Shader* m_FontShader;
    Shader* m_ShapesShader;
    Shader* m_SpriteShader;

    // Offsets into collider batch
    GLBatchDrawCmd m_EllipsoidColliderDrawCmd;

    CglFBO* m_2dFBO;
    CglFBO* m_3dFBO;
    CglFBO* m_3dFirstPersonViewFBO;
    CglFBO* m_ConsoleFBO;
    int     m_WindowWidth;
    int     m_WindowHeight;

    // 2D Rendering state
    CFont* m_CurrentFont;

    // Lightmap
    bool     m_UseLightmap = false;
    uint64_t m_hLightmapTexture;

    uint32_t m_DrawWireframe = 0;
};

#endif
