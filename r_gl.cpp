#include "r_gl.h"

#include <stdint.h>

#include <SDL.h>
#include <SDL_egl.h>
#include <glad/glad.h>

#include "globals.h"
#include "stb_truetype.h"

#include "imgui.h"
#include <SDL2/SDL_egl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"

#include "Console/Console.h"
#include "Console/VariableManager.h"
#include "Entity/base_game_entity.h"
#include "globals.h"
#include "input.h"
#include "platform.h"
#include "r_common.h"
#include "r_gl_batch.h"
#include "r_gl_fbo.h"
#include "r_gl_texture.h"
#include "r_gl_texture_mgr.h"
#include "r_itexture.h"

const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;

ConsoleVariable scr_consize      = { "scr_consize", 0.45f };
ConsoleVariable scr_conopacity   = { "scr_conopacity", 0.95f };
ConsoleVariable scr_conwraplines = { "scr_conwraplines", 1 };

void GLAPIENTRY OpenGLDebugCallback(GLenum        source,
                                    GLenum        type,
                                    GLuint        id,
                                    GLenum        severity,
                                    GLsizei       length,
                                    const GLchar* message,
                                    const void*   userParam) {
    // Ignore non-significant error/warning codes (e.g., vendor-specific warnings)
    if ( id == 131169 || id == 131185 || id == 131218 || id == 131204 ) return;

    printf("--------------------- OpenGL Debug Output ---------------------\n");
    printf("Message: %s\n", message);

    printf("Source: ");
    switch ( source ) {
    case GL_DEBUG_SOURCE_API:
        printf("API\n");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        printf("Window System\n");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        printf("Shader Compiler\n");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        printf("Third Party\n");
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        printf("Application\n");
        break;
    case GL_DEBUG_SOURCE_OTHER:
        printf("Other\n");
        break;
    }

    printf("Type: ");
    switch ( type ) {
    case GL_DEBUG_TYPE_ERROR:
        printf("Error\n");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        printf("Deprecated Behaviour\n");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        printf("Undefined Behaviour\n");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        printf("Portability\n");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        printf("Performance\n");
        break;
    case GL_DEBUG_TYPE_MARKER:
        printf("Marker\n");
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        printf("Push Group\n");
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        printf("Pop Group\n");
        break;
    case GL_DEBUG_TYPE_OTHER:
        printf("Other\n");
        break;
    }

    printf("Severity: ");
    switch ( severity ) {
    case GL_DEBUG_SEVERITY_HIGH:
        printf("High\n");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        printf("Medium\n");
        break;
    case GL_DEBUG_SEVERITY_LOW:
        printf("Low\n");
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        printf("Notification\n");
        break;
    }
    printf("---------------------------------------------------------------\n");
}

static void EnableOpenGLDebugCallback() {
    // Enable debug output (OpenGL 4.3 or higher is required)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Make sure OpenGL calls this callback in the same thread

    // Register the debug callback function
    glDebugMessageCallback(OpenGLDebugCallback, NULL);

    // You can optionally control which types of messages are logged
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

void GLRender::Shutdown(void) {
    // Deinit ImGui

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Close and destroy the window

    SDL_GL_DeleteContext(m_SDL_GL_Conext);
    SDL_DestroyWindow(m_Window);

    m_ModelBatch->Kill();
    delete m_ModelBatch;

    m_ImPrimitiveBatch->Kill();
    delete m_ImPrimitiveBatch;

    m_FontBatch->Kill();
    delete m_FontBatch;

    m_ShapesBatch->Kill();
    delete m_ShapesBatch;

    m_WorldBatch->Kill();
    delete m_WorldBatch;

    m_BrushBatch->Kill();
    delete m_BrushBatch;

    m_FontShader->Unload();

    m_ModelShader->Unload();
    delete m_ModelShader;

    m_ImPrimitivesShader->Unload();
    delete m_ImPrimitivesShader;

    // Destroy FBOs
    delete m_2dFBO;
    delete m_3dFBO;
    delete m_ConsoleFBO;
}

bool GLRender::Init(void) {
    SDL_Init(SDL_INIT_EVERYTHING);

    // From 2.0.18: Enable native IME.
    /*
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
*/

    // Create an application window with the following settings:
    m_Window = SDL_CreateWindow("HKD", 1, 1, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

    // BEWARE! These flags must be set AFTER SDL_CreateWindow. Otherwise SDL
    // just doesn't cate about them (at least on Linux)!

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_SDL_GL_Conext = SDL_GL_CreateContext(m_Window);
    if ( !m_SDL_GL_Conext ) {
        SDL_Log("Unable to create GL context! SDL-Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(m_Window, m_SDL_GL_Conext);

    int majorVersion;
    int minorVersion;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &majorVersion);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minorVersion);
    printf("OpenGL Version active: %d.%d\n", majorVersion, minorVersion);

    if ( !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) ) {
        SDL_Log("Failed to get OpenGL function pointers via GLAD: %s\n", SDL_GetError());
        return false;
    }

    printf("OpenGL Version active (via glGetString): %s\n", glGetString(GL_VERSION));

    // Allow OpenGL to send us debug information.
    EnableOpenGLDebugCallback();

    // Check that the window was successfully created

    if ( m_Window == NULL ) {
        // In the case that the window could not be made...
        SDL_Log("Could not create window: %s\n", SDL_GetError());
        return false;
    }

    SDL_ShowWindow(m_Window);

    // GL Vsync on
    if ( SDL_GL_SetSwapInterval(1) != 0 ) {
        SDL_Log("Failed to enable vsync!\n");
    } else {
        SDL_Log("vsync enabled\n");
    }

    // Init TextureManager

    m_TextureManager = GLTextureManager::Instance();

    // Setup Imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_SDL_GL_Conext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ImGui Config

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Some OpenGL global settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);

    // Some OpenGL Info

    const GLubyte* vendor   = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    SDL_Log("%s, %s\n", vendor, renderer);

    // Create batches

    // make space for 1Mio vertices
    // TODO: What should be the upper limit?
    // TODO: Shouldn't we just pass in the max size in bytes for GLBatch Ctor?
    // With sizeof(Vertex) = 92bytes => sizeof(Tri) = 276bytes we need ~ 263MB for Models.
    // A lot for a game in the 2000s! Our models have a tri count of maybe 3000 Tris (without weapon), which
    // is not even close to 1Mio tris.
    m_ModelBatch = new GLBatch(500 * 1000);

    // Batches but for different purposes
    m_ImPrimitiveBatch        = new GLBatch(1000);
    m_ImPrimitiveBatchIndexed = new GLBatch(1000, 1000);
    m_ColliderBatch           = new GLBatch(1000);
    m_FontBatch               = new GLBatch(1000, 1000);
    m_ShapesBatch             = new GLBatch(1000, 1000);
    m_WorldBatch              = new GLBatch(100000);
    m_BrushBatch              = new GLBatch(50000);

    // Initialize shaders

    InitShaders();

    // Create and upload Collider Models to GPU

    RegisterColliderModels();

    // Create FBOs for 3D/2D Rendering

    // FBO for rendering the 3d scene.
    m_3dFBO = new CglFBO(WINDOW_WIDTH, WINDOW_HEIGHT);
    // FBO for rendering text and other 2d elements (shapes, sprites, ...)
    // on top of the 3d scene.
    m_2dFBO = new CglFBO(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_ConsoleFBO = new CglFBO(WINDOW_WIDTH, WINDOW_HEIGHT);

    VariableManager::Register(&scr_consize);
    VariableManager::Register(&scr_conopacity);
    VariableManager::Register(&scr_conwraplines);

    return true;
}

// At the moment we don't generate a drawCmd for a model. We just but all of
// the meshes into a GPU buffer and draw all of them exactly the same way.
int GLRender::RegisterModel(HKD_Model* model) {
    GLModel gl_model = {};
    int     offset   = m_ModelBatch->Add(&model->tris[ 0 ], model->tris.size());

    for ( int i = 0; i < model->meshes.size(); i++ ) {
        HKD_Mesh*  mesh    = &model->meshes[ i ];
        GLTexture* texture = (GLTexture*)m_TextureManager->CreateTexture(mesh->textureFileName);
        GLMesh     gl_mesh
            = { .triOffset = offset / 3 + (int)mesh->firstTri, .triCount = (int)mesh->numTris, .texture = texture };
        gl_model.meshes.push_back(gl_mesh);
    }

    m_Models.push_back(gl_model);

    int gpuModelHandle    = m_Models.size() - 1;
    model->gpuModelHandle = gpuModelHandle;

    return gpuModelHandle;
}

// FIX: Same as RegisterModel just with a different batch!
int GLRender::RegisterBrush(HKD_Model* model) {
    GLModel gl_model = {};
    int     offset   = m_BrushBatch->Add(&model->tris[ 0 ], model->tris.size());

    for ( int i = 0; i < model->meshes.size(); i++ ) {
        HKD_Mesh*  mesh    = &model->meshes[ i ];
        GLTexture* texture = (GLTexture*)m_TextureManager->CreateTexture(mesh->textureFileName);
        GLMesh     gl_mesh
            = { .triOffset = offset / 3 + (int)mesh->firstTri, .triCount = (int)mesh->numTris, .texture = texture };
        gl_model.meshes.push_back(gl_mesh);
    }

    m_Models.push_back(gl_model);

    int gpuModelHandle    = m_Models.size() - 1;
    model->gpuModelHandle = gpuModelHandle;

    return gpuModelHandle;
}

void GLRender::RegisterFont(CFont* font) {
    GLTexture* texture = (GLTexture*)m_TextureManager->CreateTexture(font);
}

void GLRender::RegisterWorld(CWorld* world) {
    std::vector<MapTri>& tris          = world->GetMapTris();
    uint64_t             numStaticTris = world->StaticGeometryCount();

    // Sort static Tris by texture
    std::unordered_map<uint64_t, std::vector<MapTri>> texHandle2Tris{};

    for ( int i = 0; i < numStaticTris; i++ ) {
        MapTri pTri = tris[ i ];
        if ( texHandle2Tris.contains(pTri.hTexture) ) {
            std::vector<MapTri>& triList = texHandle2Tris.at(pTri.hTexture);
            triList.push_back(pTri);
        } else {
            std::vector<MapTri> newTriList{ pTri };
            texHandle2Tris.insert({ pTri.hTexture, newTriList });
        }
    }

    // Upload tris to GPU and generate draw cmds.

    // FIX: ALL tris are registered here. Brush entities should go into a dedicated dynamic batch.
    for ( auto& [ texHandle, triList ] : texHandle2Tris ) {
        std::vector<MapTri> pTris = triList;
        int vertexOffset          = m_WorldBatch->AddMapTris(pTris.data(), triList.size(), true, DRAW_MODE_SOLID);
        assert(vertexOffset >= 0 && "Tried to add Tris to World Batch but it returned a negative offset!");
        GLBatchDrawCmd drawCmd = { vertexOffset,       // Vertex Buffer offset is the current vert count of the batch
                                   0,                  // Index buffer offset.  Ignored: World is drawn without indices
                                   triList.size() * 3, // Num vertices (3 per tri)
                                   0,                  // Num indices: irgnored
                                   true,               // Cull back faces
                                   DRAW_MODE_SOLID };
        m_TexHandleToWorldDrawCmd.insert({ texHandle, drawCmd });
    }
}

// Returns the CPU handle
uint64_t GLRender::RegisterTextureGetHandle(std::string name) {
    return m_TextureManager->CreateTextureGetHandle(name);
}

void GLRender::SetActiveCamera(Camera* camera) {
    m_ActiveCamera = camera;
}

void GLRender::RegisterColliderModels() {
    // Generate vertices for a circle. Used for ellipsoid colliders.

    MeshEllipsoid unitEllipsoid = CreateUnitEllipsoid(2);

    m_EllipsoidColliderDrawCmd = AddTrisToBatch(
        m_ColliderBatch, unitEllipsoid.tris.data(), unitEllipsoid.tris.size(), false, DRAW_MODE_WIREFRAME);
}

// Maybe return a void* as GPU handle, because usually APIs that use the handle of
// a specific graphics API don't expect it to be in int or whatever.
std::vector<ITexture*> GLRender::ModelTextures(int gpuModelHandle) {
    std::vector<ITexture*> results;

    if ( gpuModelHandle >= m_Models.size() )
        return results;
    else if ( gpuModelHandle < 0 )
        return results;

    GLModel* model = &m_Models[ gpuModelHandle ];
    for ( auto& mesh : model->meshes ) {
        results.push_back(mesh.texture);
    }

    return results;
}

std::vector<ITexture*> GLRender::Textures(void) {
    std::vector<ITexture*> result;
    for ( auto& elem : m_TextureManager->m_NameToTexture ) {
        result.push_back(elem.second);
    }

    return result;
}

void GLRender::ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode) {
    int offset = m_ImPrimitiveBatch->Add(tris, numTris, cullFace, drawMode);

    GLBatchDrawCmd drawCmd = { .offset = offset, .numVerts = 3 * numTris, .cullFace = cullFace, .drawMode = drawMode };

    m_PrimitiveDrawCmds.push_back(drawCmd);
}

void GLRender::ImDrawTriPlanes(TriPlane* triPlanes, uint32_t numTriPlanes, bool cullFace, DrawMode drawMode) {
    std::vector<Tri> tris;
    tris.resize(numTriPlanes);
    for ( int i = 0; i < numTriPlanes; i++ ) {
        tris[ i ] = triPlanes[ i ].tri;
    }
    ImDrawTris(tris.data(), numTriPlanes, cullFace, drawMode);
}

GLBatchDrawCmd GLRender::AddTrisToBatch(GLBatch* batch, Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode) {
    int offset = batch->Add(tris, numTris, cullFace, drawMode);

    GLBatchDrawCmd drawCmd = { .offset = offset, .numVerts = 3 * numTris, .cullFace = cullFace, .drawMode = drawMode };

    return drawCmd;
}

// Draw triangles with indexed geometry :)
void GLRender::ImDrawIndexed(
    Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, bool cullFace, DrawMode drawMode) {
    int offset        = 0;
    int offsetIndices = 0;
    if ( !m_ImPrimitiveBatchIndexed->Add(
             verts, numVerts, indices, numIndices, &offset, &offsetIndices, cullFace, drawMode) ) {
        return;
    }

    GLBatchDrawCmd drawCmd = { .offset      = offset,
                               .indexOffset = offsetIndices,
                               .numVerts    = numVerts,
                               .numIndices  = numIndices,
                               .cullFace    = cullFace,
                               .drawMode    = drawMode };

    m_PrimitiveIndexdDrawCmds.push_back(drawCmd);
}

// TODO: not done
void GLRender::ImDrawVerts(Vertex* verts, uint32_t numVerts) {
    int offset = m_ImPrimitiveBatch->Add(verts, numVerts);

    GLBatchDrawCmd drawCmd = { .offset = offset, .numVerts = numVerts, .cullFace = false, .drawMode = DRAW_MODE_SOLID };
}

void GLRender::ImDrawCircle(glm::vec3 center, float radius, glm::vec3 normal) {
    int                 segments = 32;
    std::vector<Vertex> circleVerts;
    float               step        = glm::two_pi<float>() / (float)segments;
    glm::mat3           rotationMat = glm::mat3(1.0);
    if ( normal != DOD_WORLD_UP ) {
        glm::vec3 axis = glm::cross(DOD_WORLD_UP, glm::normalize(normal));

        float angle = glm::acos(glm::dot(DOD_WORLD_UP, glm::normalize(normal)));
        rotationMat = glm::rotate(glm::mat4(1), angle, glm::normalize(axis));
    }

    circleVerts.reserve(segments);
    for ( int i = 0; i < segments; i++ ) {
        glm::vec3 position = glm::vec3(radius * glm::cos((float)i * step), radius * glm::sin((float)i * step), 0.0f);
        position           = center + rotationMat * position;

        circleVerts.push_back({
            .pos    = position,
            .normal = normal,
        });
    }

    ImDrawLines(circleVerts.data(), circleVerts.size(), true);
}

// We have separate draw cmds from the batch. This function generate unneccessary many
// draw cmds (each Add is a new one!).
void GLRender::ImDrawLines(Vertex* verts, uint32_t numVerts, bool close) {
    if ( numVerts < 2 ) { // This won't work, man.
        return;
    }

    Vertex* v = verts;
    // TODO: DRAW_MODEL_LINES doesn't do anything to the batch!
    int offset = m_ImPrimitiveBatch->Add(v, 2, false, DRAW_MODE_LINES);
    v += 1;
    int moreVerts = 0;
    for ( int i = 2; i < numVerts; i++ ) {
        m_ImPrimitiveBatch->Add(v, 2, false, DRAW_MODE_LINES);
        v++;
        moreVerts += 1;
    }

    if ( close ) {
        Vertex endAndStart[] = { *v, verts[ 0 ] };
        m_ImPrimitiveBatch->Add(endAndStart, 2, false, DRAW_MODE_LINES);
        moreVerts += 2;
    }

    GLBatchDrawCmd drawCmd
        = { .offset = offset, .numVerts = numVerts + moreVerts, .cullFace = false, .drawMode = DRAW_MODE_LINES };

    m_PrimitiveDrawCmds.push_back(drawCmd);
}

void GLRender::ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color) {
    glm::mat4 view = m_ActiveCamera->ViewMatrix();
    // TODO: Global Setting for perspective values
    glm::mat4 proj
        = glm::perspective(glm::radians(45.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 10000.0f);

    m_ColliderShader->Activate();
    m_ColliderShader->SetViewProjMatrices(view, proj);

    m_ColliderBatch->Bind();
    m_ColliderShader->SetVec4("uDebugColor", color);
    glm::vec3 scale = glm::vec3(radius);
    glm::mat4 T     = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 S     = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 M     = T * S;
    m_ColliderShader->SetMat4("model", M);

    glDrawArrays(GL_TRIANGLES, m_EllipsoidColliderDrawCmd.offset, m_EllipsoidColliderDrawCmd.numVerts);
}

GLBatchDrawCmd GLRender::AddLineToBatch(GLBatch* batch, Vertex* verts, uint32_t numVerts, bool close) {
    if ( numVerts < 2 ) { // This won't work, man.
        return { -1 };
    }

    Vertex* v = verts;
    // TODO: DRAW_MODEL_LINES doesn't do anything to the batch!
    int offset = batch->Add(v, 2, false, DRAW_MODE_LINES);
    v += 1;
    int moreVerts = 0;
    for ( int i = 2; i < numVerts; i++ ) {
        batch->Add(v, 2, false, DRAW_MODE_LINES);
        v++;
        moreVerts += 1;
    }

    if ( close ) {
        Vertex endAndStart[] = { *v, verts[ 0 ] };
        batch->Add(endAndStart, 2, false, DRAW_MODE_LINES);
        moreVerts += 2;
    }

    GLBatchDrawCmd drawCmd
        = { .offset = offset, .numVerts = numVerts + moreVerts, .cullFace = false, .drawMode = DRAW_MODE_LINES };

    return drawCmd;
}

void GLRender::RenderBegin(void) {
    // Make sure the screenspace 2d FBO is being cleared.
    m_2dFBO->Bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // NOTE: Important to have alpha = 0.0f, otherwise we will end up with
    // a base framebuffer that is fully opaque. But only the glyphs must
    // be opaque after the fragment shader has run!
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glViewport(0, 0, m_2dFBO->m_Width, m_2dFBO->m_Height);
    m_2dFBO->Unbind(); // Switch back to GL default framebuffer.

    // Make sure the console FBO is being cleared.
    m_ConsoleFBO->Bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_2dFBO->m_Width, m_2dFBO->m_Height);
    m_ConsoleFBO->Unbind(); // Switch back to GL default framebuffer.

    // Render into the GL default FBO.

    // See: https://wiki.libsdl.org/SDL2/SDL_GL_GetDrawableSize
    SDL_GL_GetDrawableSize(m_Window, &m_WindowWidth, &m_WindowHeight);
    float windowAspect = (float)m_WindowWidth / (float)m_WindowHeight;
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
}

void GLRender::Begin3D(void) {
    // Render into the 3D scene FBO
    m_3dFBO->Bind();
    SDL_GL_GetDrawableSize(m_Window, &m_WindowWidth, &m_WindowHeight);
    float windowAspect = (float)m_WindowWidth / (float)m_WindowHeight;
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    glClearColor(0.f, 0.f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRender::End3D(void) {
    m_3dFBO->Unbind(); // Set state back to GL default FBO.
}

void GLRender::ExecuteDrawCmds(std::vector<GLBatchDrawCmd>& drawCmds, GeometryType geomType) {
    uint32_t prevDrawMode  = GL_FILL;
    uint32_t primitiveType = GL_TRIANGLES;
    for ( int i = 0; i < drawCmds.size(); i++ ) {

        GLBatchDrawCmd drawCmd = drawCmds[ i ];

        if ( !drawCmd.cullFace ) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }

        if ( prevDrawMode != drawCmd.drawMode ) {
            if ( drawCmd.drawMode == DRAW_MODE_WIREFRAME ) {
                glLineWidth(1.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                m_ImPrimitivesShader->SetShaderSettingBits(SHADER_LINEMODE);
                prevDrawMode  = GL_LINE;
                primitiveType = GL_TRIANGLES;
            } else if ( drawCmd.drawMode == DRAW_MODE_LINES ) {
                glLineWidth(5.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                m_ImPrimitivesShader->SetShaderSettingBits(SHADER_LINEMODE);
                prevDrawMode  = GL_FILL;
                primitiveType = GL_LINES;
            } else { // DRAW_MODE_SOLID
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                prevDrawMode  = GL_FILL;
                primitiveType = GL_TRIANGLES;
            }
        }

        if ( geomType == GEOM_TYPE_VERTEX_ONLY ) {
            glDrawArrays(primitiveType, drawCmd.offset, drawCmd.numVerts);
        } else if ( geomType == GEOM_TYPE_INDEXED ) {
            uint32_t indexBufferUSOffset = drawCmd.indexOffset * sizeof(uint16_t);
            glDrawElementsBaseVertex(primitiveType,
                                     drawCmd.numIndices,
                                     GL_UNSIGNED_SHORT,
                                     (GLvoid*)(drawCmd.indexOffset * sizeof(uint16_t)),
                                     drawCmd.offset);
        }

        m_ImPrimitivesShader->ResetShaderSettingBits(SHADER_LINEMODE);
    }
}

void GLRender::Render(
    Camera* camera, HKD_Model** models, uint32_t numModels, HKD_Model** brushModels, uint32_t numBrushModels) {
    // Camera and render settings

    static uint32_t drawWireframe = 0;

    if ( KeyWentDown(SDLK_r) ) { // WARNING: TAB key also triggers slider-values in ImGui Window.
        drawWireframe ^= 1;
    }

    ImGui::Begin("controlls");
    ImGui::Text("Cam position:");
    ImGui::SliderFloat("x", &camera->m_Pos.x, -500.0f, 500.0f);
    ImGui::SliderFloat("y", &camera->m_Pos.y, -500.0f, 500.0f);
    ImGui::SliderFloat("z", &camera->m_Pos.z, -500.0f, 500.0f);
    ImGui::Text("Render settings:");
    ImGui::Checkbox("wireframe", (bool*)&drawWireframe);
    ImGui::End();

    glm::mat4 view = camera->ViewMatrix();
    // TODO: Global Setting for perspective values
    glm::mat4 proj
        = glm::perspective(glm::radians(45.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 10000.0f);

    // Draw immediate mode primitives

    // non indexed

    m_ImPrimitiveBatch->Bind();
    m_ImPrimitivesShader->Activate();
    m_ImPrimitivesShader->DrawWireframe((uint32_t)drawWireframe);
    m_ImPrimitivesShader->SetViewProjMatrices(view, proj);
    m_ImPrimitivesShader->SetMat4("model", glm::mat4(1));
    m_ImPrimitivesShader->SetVec3("viewPos", camera->m_Pos);
    ExecuteDrawCmds(m_PrimitiveDrawCmds, GEOM_TYPE_VERTEX_ONLY);

    // indexed

    m_ImPrimitiveBatchIndexed->Bind();

    // TODO: Maybe we should have dedicated functions for indexed vs non-indexed
    ExecuteDrawCmds(m_PrimitiveIndexdDrawCmds, GEOM_TYPE_INDEXED);

    // Reset to default state

    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);

    // Draw World Tris
    m_WorldBatch->Bind();
    m_WorldShader->Activate();
    m_WorldShader->SetViewProjMatrices(view, proj);

    for ( auto const& [ texHandle, drawCmd ] : m_TexHandleToWorldDrawCmd ) {
        std::vector<GLBatchDrawCmd> drawCmds{ drawCmd };
        glBindTexture(GL_TEXTURE_2D, texHandle);
        ExecuteDrawCmds(drawCmds, GEOM_TYPE_VERTEX_ONLY);
    }

    /*for (auto const& [ texHandle, batch ] : m_TexHandleToWorldBatch) {*/
    /*    batch->Bind();*/
    /*    glBindTexture( GL_TEXTURE_2D, texHandle );*/
    /*    glDrawArrays( GL_TRIANGLES, 0, batch->VertCount() );*/
    /*}*/

    // Draw Brush Models

    m_BrushBatch->Bind();
    m_BrushShader->Activate();
    m_BrushShader->SetViewProjMatrices(view, proj);
    for ( int i = 0; i < numBrushModels; i++ ) {
        GLModel   model       = m_Models[ brushModels[ i ]->gpuModelHandle ];
        glm::mat4 modelMatrix = CreateModelMatrix(brushModels[ i ]);
        m_BrushShader->SetVec3("position", brushModels[ i ]->position);

        for ( int j = 0; j < model.meshes.size(); j++ ) {
            GLMesh* mesh = &model.meshes[ j ];
            glBindTexture(GL_TEXTURE_2D, mesh->texture->m_gl_Handle);
            glDrawArrays(GL_TRIANGLES, 3 * mesh->triOffset, 3 * mesh->triCount);
        }
    }

    // Draw Models

    m_ModelBatch->Bind();
    m_ModelShader->Activate();
    m_ModelShader->SetViewProjMatrices(view, proj);
    if ( drawWireframe ) {
        m_ModelShader->SetShaderSettingBits(SHADER_WIREFRAME_ON_MESH);
    } else {
        m_ModelShader->ResetShaderSettingBits(SHADER_WIREFRAME_ON_MESH);
    }
    for ( int i = 0; i < numModels; i++ ) {

        HKD_Model* hkdModel = models[ i ];

        if ( hkdModel->renderFlags & MODEL_RENDER_FLAG_IGNORE ) {
            continue;
        }

        GLModel model = m_Models[ hkdModel->gpuModelHandle ];

        if ( hkdModel->numJoints > 0 ) {
            m_ModelShader->SetShaderSettingBits(SHADER_ANIMATED);
            m_ModelShader->SetMatrixPalette(&hkdModel->palette[ 0 ], hkdModel->numJoints);
        } else {
            m_ModelShader->ResetShaderSettingBits(SHADER_ANIMATED);
        }

        BaseGameEntity* pOwner           = hkdModel->pOwner;
        glm::vec3       ownerPos         = glm::vec3(0.0f);
        glm::quat       ownerOrientation = glm::angleAxis(0.0f, DOD_WORLD_FORWARD);
        if ( pOwner != nullptr ) {
            ownerPos         = pOwner->m_Position;
            ownerOrientation = pOwner->m_Orientation;
        }
        glm::vec3 position    = ownerPos + hkdModel->position;
        glm::quat orientation = ownerOrientation * hkdModel->orientation;
        glm::vec3 scale       = hkdModel->scale;
        glm::mat4 modelMatrix = CreateModelMatrix(position, orientation, scale);
        m_ModelShader->SetMat4("model", modelMatrix);

        for ( int j = 0; j < model.meshes.size(); j++ ) {
            GLMesh* mesh = &model.meshes[ j ];
            if ( !mesh->texture->m_Filename.empty() ) { // TODO: Checking string of empty is not great.
                m_ModelShader->SetShaderSettingBits(SHADER_IS_TEXTURED);
                glBindTexture(GL_TEXTURE_2D, mesh->texture->m_gl_Handle);
            } else {
                m_ModelShader->ResetShaderSettingBits(SHADER_IS_TEXTURED);
            }
            glDrawArrays(GL_TRIANGLES, 3 * mesh->triOffset, 3 * mesh->triCount);
        }
    }
    m_ModelShader->ResetShaderSettingBits(SHADER_ANIMATED);

    //const std::vector<GLBatchDrawCmd>& modelDrawCmds = m_ModelBatch->DrawCmds();
    //for (int i = 0; i < modelDrawCmds.size(); i++) {
    //    glBindTexture(GL_TEXTURE_2D, modelDrawCmds[i].hTexture);
    //    glDrawArrays(GL_TRIANGLES, 3*modelDrawCmds[i].offset, 3 * modelDrawCmds[i].numTris);
    //}
    //glDrawArrays(GL_TRIANGLES, 0, 3*m_ModelBatch->TriCount());
}

void GLRender::DrawWorldTris() {
    // TODO: This is actually done in the main render 3D function.
    // Things are in flux so maybe this function will be removed
    // or renamed or...
}

// Draw 2d screenspace elements
void GLRender::Begin2D() {

    m_2dFBO->Bind();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE); // necessary: text is drawn backwards and allows drawing shapes with negative width/height

    glViewport(0, 0, m_2dFBO->m_Width, m_2dFBO->m_Height);

    glm::mat4 ortho = glm::ortho(0.0f, (float)m_2dFBO->m_Width, (float)m_2dFBO->m_Height, 0.0f, -1.0f, 1.0f);
    m_FontShader->Activate();
    m_FontShader->SetViewProjMatrices(glm::mat4(1.0f), ortho);
    m_ShapesShader->Activate();
    m_ShapesShader->SetViewProjMatrices(glm::mat4(1.0f), ortho);
}

void GLRender::End2D() {

    m_2dFBO->Unbind();
    //m_FontBatch->Unbind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // TODO: (Michael): Unbind bound (font-)textures?
}

void GLRender::SetFont(CFont* font, glm::vec4 color) {

    m_FontShader->Activate();

    ITexture* fontTexture = m_TextureManager->GetTexture(font->m_Filename);
    glBindTexture(GL_TEXTURE_2D, (GLuint)fontTexture->m_hGPU);

    FontUB fontShaderData = {
        color,
        glm::vec4(0.0f) // NOTE: unused at the moment.
    };

    glBindBuffer(GL_UNIFORM_BUFFER, m_FontShader->m_FontUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FontUB), (void*)&fontShaderData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_CurrentFont = font;
}

void GLRender::SetShapeColor(glm::vec4 color) {
    m_ShapesShader->Activate();

    ShapesUB shapesShaderData = {
        color,
        glm::vec4(0.0f) // NOTE: unused at the moment.
    };

    glBindBuffer(GL_UNIFORM_BUFFER, m_ShapesShader->m_ShapesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShapesUB), (void*)&shapesShaderData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLRender::FlushFonts() {
    m_FontBatch->Bind();
    m_FontShader->Activate();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_FontBatch->IndexCount(), GL_UNSIGNED_SHORT, (GLvoid*)0, 0);

    m_FontBatch->Reset();
}

void GLRender::FlushShapes() {
    m_ShapesBatch->Bind();
    m_ShapesShader->Activate();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ShapesBatch->IndexCount(), GL_UNSIGNED_SHORT, (GLvoid*)0, 0);

    m_ShapesBatch->Reset();
}

void GLRender::R_DrawText(const std::string& text, float x, float y, ScreenSpaceCoordMode coordMode) {

    // TODO: (Michael): Make sure that the correct shader is active.

    float xOffset = WINDOW_WIDTH * x;
    float yOffset = WINDOW_HEIGHT * y;

    if ( coordMode == COORD_MODE_ABS ) {
        xOffset = x;
        yOffset = y;
    }

    FaceQuad fq{}; // = CreateFaceQuadFromVerts(vertices);

    // go through each character in text and lookup the correct UV.
    int out_OffsetIndices  = 0; // TODO: Not needed here!
    int out_OffsetVertices = 0; // TODO: Not needed here!

    // TODO: (Michael): Make indices uint32_t.
    uint16_t iOffset   = (uint16_t)m_FontBatch->m_LastIndex;
    uint16_t lastIndex = iOffset;

    const char* c            = text.c_str();
    int         i            = 0;
    float       ascender     = (float)m_CurrentFont->m_Ascender;
    float       tallestGlyph = ascender;
    while ( *c != '\0' ) {
        if ( *c >= FIRST_CODE_POINT && *c < LAST_CODE_POINT ) {

            uint16_t           indices[ 6 ] = { iOffset + 0 + i * 4, iOffset + 1 + i * 4, iOffset + 2 + i * 4,
                                                iOffset + 2 + i * 4, iOffset + 3 + i * 4, iOffset + 0 + i * 4 };
            stbtt_aligned_quad q;

            // Newbe API
            //stbtt_GetBakedQuad(m_CurrentFont->m_Cdata, 512, 512, *c-32, &currentX, &y, &q, 1);//1=opengl & d3d10+,0=d3d9

            // More advanced API
            stbtt_GetPackedQuad(m_CurrentFont->m_PackedCharData,
                                FONT_TEX_SIZE,
                                FONT_TEX_SIZE,
                                *c - FIRST_CODE_POINT, // character to display
                                &xOffset,
                                &yOffset,
                                // pointers to current position in screen pixel space
                                &q, // output: quad to draw
                                1);

            float x0 = q.x0;
            float y0 = q.y0;
            float x1 = q.x1;
            float y1 = q.y1;

            // NOTE: For some reason the positional
            // coordinates in aligned_quad are flipped vertically. Not sure why.
            fq.c.pos = { x0, y0 + tallestGlyph, 0.0f };
            fq.d.pos = { x1, y0 + tallestGlyph, 0.0f };
            fq.a.pos = { x1, y1 + tallestGlyph, 0.0f };
            fq.b.pos = { x0, y1 + tallestGlyph, 0.0f };
            fq.a.uv  = { q.s1, q.t1 };
            fq.b.uv  = { q.s0, q.t1 };
            fq.c.uv  = { q.s0, q.t0 };
            fq.d.uv  = { q.s1, q.t0 };
            m_FontBatch->Add(
                fq.vertices, 4, indices, 6, &out_OffsetVertices, &out_OffsetIndices, false, DRAW_MODE_SOLID);

            lastIndex = iOffset + 3 + i * 4;

            i++;
        }
        c++;
    }

    // We need one *after* this batche's data for the next batch.
    m_FontBatch->m_LastIndex = lastIndex + 1;

    // TODO: See DrawBox() function!
    FlushFonts();
}

void GLRender::DrawBox(float x, float y, float width, float height, ScreenSpaceCoordMode coordMode) {

    float x0 = WINDOW_WIDTH * x;
    float y0 = WINDOW_HEIGHT * y;
    float x1 = WINDOW_WIDTH * (x + width);
    float y1 = WINDOW_HEIGHT * (y + height);

    if ( coordMode == COORD_MODE_ABS ) {
        x0 = x;
        y0 = y;
        x1 = x + width;
        y1 = y + height;
    }

    Vertex   verts[ 4 ] = { { glm::vec3(x0, y0, 0.0f) },
                            { glm::vec3(x0, y1, 0.0f) },
                            { glm::vec3(x1, y1, 0.0f) },
                            { glm::vec3(x1, y0, 0.0f) } };
    FaceQuad fq         = CreateFaceQuadFromVerts(verts);

    uint16_t iOffset      = (uint16_t)m_ShapesBatch->m_LastIndex;
    uint16_t indices[ 6 ] = { iOffset + 0, iOffset + 1, iOffset + 2, iOffset + 2, iOffset + 3, iOffset + 0 };

    // NOTE: (Michael): The batch interface is going to change once we have
    // the first prototype going. So no need to fix those inconvenciences
    // of unused (and unwanted) out_* variables for now!
    int out_OffsetIndices  = 0; // TODO: Not needed here!
    int out_OffsetVertices = 0; // TODO: Not needed here!
    m_ShapesBatch->Add(fq.vertices, 4, indices, 6, &out_OffsetVertices, &out_OffsetIndices, false, DRAW_MODE_SOLID);

    m_ShapesBatch->m_LastIndex += 4;

    // TODO: We can batch multiple calls to DrawBox together but this
    // also makes the renderer more complicated! We *really* need to
    // keep things simple so we don't fall over our own abstractions.
    // Especially because so many variables are unknown to us at this point!
    // We will see what we can optimize when we actually have gameplay
    // code and a running game! One could argue that the batching system
    // in place is already too high-level!
    FlushShapes();
}

void GLRender::RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels) {
    glm::mat4 view = camera->ViewMatrix();
    // TODO: Global Setting for perspective values
    glm::mat4 proj
        = glm::perspective(glm::radians(45.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 10000.0f);

    m_ColliderShader->Activate();
    m_ColliderShader->SetViewProjMatrices(view, proj);

    m_ColliderBatch->Bind();
    for ( int i = 0; i < numModels; i++ ) {
        HKD_Model* pModel   = models[ i ];
        glm::vec3  ownerPos = glm::vec3(0.0f);
        if ( pModel->pOwner != nullptr ) {
            ownerPos = pModel->pOwner->m_Position;
        }
        m_ColliderShader->SetVec4("uDebugColor", pModel->debugColor);
        EllipsoidCollider ec    = pModel->ellipsoidColliders[ pModel->currentAnimIdx ];
        glm::vec3         scale = glm::vec3(ec.radiusA, ec.radiusA, ec.radiusB);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), ownerPos);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 M = T * S;
        m_ColliderShader->SetMat4("model", M);
        std::vector<GLBatchDrawCmd> drawCmds = { m_EllipsoidColliderDrawCmd };
        //glDrawArrays(GL_TRIANGLES, m_EllipsoidColliderDrawCmd.offset, m_EllipsoidColliderDrawCmd.numVerts);
        // FIX: Add to the list of draw commands and execute later? Not sure...
        ExecuteDrawCmds(drawCmds, GEOM_TYPE_VERTEX_ONLY);
        //m_PrimitiveDrawCmds.push_back(m_EllipsoidColliderDrawCmd);
        //glDrawArrays(GL_LINES,
        //             m_EllipsoidColliderDrawCmd.offset,
        //             m_EllipsoidColliderDrawCmd.numVerts);
    }
}

void GLRender::RenderConsole(Console* console, CFont* font) {
    if ( !console->m_isActive ) return;

    const float relHeight   = scr_consize.value;
    const float height      = m_WindowHeight * relHeight;
    const float borderWidth = 2.0f;
    const float textMargin  = 10.0f;
    const float lineHeight
        = font->m_Size + textMargin; // TODO: possibly derive from font *line gap* metric in the future?
    const float charWidth
        = font->m_Size
          * 0.602f; // assumes mono-space font  // FIXME: this is just an estimation, should be derived from the font!

    const float inputY       = height - font->m_Size - textMargin;
    float       logY         = inputY - textMargin * 2 - font->m_Size;
    const int   maxLines     = floor(logY / lineHeight) + 1;
    const int   maxChars     = floor((m_WindowWidth - 2 * textMargin) / charWidth);
    const bool  isScrollable = console->m_lineBuffer.Size() > maxLines;

    Begin2D();
    m_ConsoleFBO->Bind();
    // draw background/frame
    SetShapeColor(glm::vec4(0.05f, 0.05f, 0.05f, scr_conopacity.value));
    DrawBox(0.0f, 0.0f, 1.0f, relHeight);
    SetShapeColor(glm::vec4(1.0f));
    DrawBox(0.0f, 0.0f, borderWidth, height, COORD_MODE_ABS);
    DrawBox(m_WindowWidth, 0.0f, -borderWidth, height, COORD_MODE_ABS);
    DrawBox(0.0f, 0.0f, m_WindowWidth, borderWidth, COORD_MODE_ABS);
    DrawBox(0.0f, inputY - textMargin, m_WindowWidth, -borderWidth, COORD_MODE_ABS);
    DrawBox(0.0f, height, m_WindowWidth, -borderWidth, COORD_MODE_ABS);

    // draw input
    SetFont(font, glm::vec4(1.0f));
    std::string prefix        = "> ";
    int         maxInputChars = maxChars - prefix.length();
    int         textOffset    = ((console->CursorPos() - 1) / maxInputChars) * maxInputChars; // integer division
    if ( console->CurrentInput().length() > maxInputChars ) prefix = "<>";                    // indicate line overflow
    std::string inputText = prefix + console->CurrentInput().substr(textOffset, maxInputChars);
    R_DrawText(inputText, textMargin, inputY, COORD_MODE_ABS);

    // draw cursor
    if ( console->m_blinkTimer < 500.0 ) {
        float cursorX = textMargin + (console->CursorPos() - textOffset + prefix.length()) * charWidth;
        SetShapeColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        DrawBox(cursorX - 2.5f, inputY - 3.0f, 5.0f, font->m_Size + 4.0f, COORD_MODE_ABS);
    } else if ( console->m_blinkTimer > 1000.0 ) {
        console->m_blinkTimer = 0;
    }

    // draw bottom scroll indicator
    if ( isScrollable && console->ScrollPos() != 0 ) {
        R_DrawText(std::string(maxChars, '^'), textMargin, logY + lineHeight * 0.7f, COORD_MODE_ABS);
    }

    // draw log lines
    for ( int i = 0; i < maxLines; i++ ) {
        int         lineOffset = isScrollable ? console->ScrollPos() + i : i;
        std::string line;
        if ( !console->m_lineBuffer.Get(lineOffset, &line) ) {
            if ( isScrollable ) {
                std::string msg   = " END OF LOG ";
                int         chars = (maxChars - msg.length()) / 2;
                msg               = std::string(chars, '-') + msg;
                R_DrawText(msg + std::string(maxChars - msg.length(), '-'), textMargin, logY, COORD_MODE_ABS);
            }
            break;
        }
        if ( scr_conwraplines.value && line.length() > maxChars ) {
            std::vector<std::string> segments;
            for ( int offset = 0; offset < line.length(); offset += maxChars ) {
                segments.push_back(line.substr(offset, maxChars));
            }
            for ( int j = segments.size() - 1; j >= 0; j-- ) {
                R_DrawText(segments[ j ], textMargin, logY, COORD_MODE_ABS);
                logY -= lineHeight;
                if ( ++i == maxLines ) break;
            }
        } else {
            R_DrawText(line, textMargin, logY, COORD_MODE_ABS);
            logY -= lineHeight;
        }
    }

    m_ConsoleFBO->Unbind();
    End2D();
}

void GLRender::RenderEnd(void) {
    // At this point the GL default FBO must be active!
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // FIX: Some buffer *HAS* to be bound apparently before a draw call
    // so OpenGL doesn't freak out... So this buffer is not even used
    // duing drawing because the compositing step is completely GPU driven.
    m_FontBatch->Bind();

    // Composite all the FBOs together
    m_CompositeShader->Activate();

    GLuint texLoc3d = glGetUniformLocation(m_CompositeShader->Program(), "main3dSceneTexture");

    GLuint texLoc2d = glGetUniformLocation(m_CompositeShader->Program(), "screenspace2dTexture");

    GLuint texLocConsole = glGetUniformLocation(m_CompositeShader->Program(), "consoleTexture");

    glUniform1i(texLoc3d, 0);
    // Bind the 3d scene FBO and draw it.
    CglRenderTexture main3dSceneTexture = m_3dFBO->m_ColorTexture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, main3dSceneTexture.m_gl_Handle);

    glUniform1i(texLoc2d, 1);
    // Bind the 2d screenspace FBO texture and draw on top.
    CglRenderTexture screenSpace2dTexture = m_2dFBO->m_ColorTexture;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, screenSpace2dTexture.m_gl_Handle);

    glUniform1i(texLocConsole, 2);
    // Bind the 2d console FBO texture and draw on top.
    CglRenderTexture consoleTexture = m_ConsoleFBO->m_ColorTexture;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, consoleTexture.m_gl_Handle);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // FIX: This is the reason why textures in OpenGL suck!!! If we are
    // not careful, and *don't* set the state back, then the following
    // rendercommands for the next frame are all messed up due to the wrong
    // texture unit being set from the previous call
    // ( glActiveTexture( GL_TEXTURE1 ); It is very easy to forget
    // (I just did!). Luckily, we can use bindless
    // textures in OpenGL which we *definitely* want to use in a
    // facelift of the renderer.
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    // Render ImGui Elements ontop of everything.
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(m_Window);

    m_ImPrimitiveBatch->Reset();
    m_ImPrimitiveBatchIndexed->Reset();
    m_FontBatch->Reset();
    m_ShapesBatch->Reset();
    m_PrimitiveDrawCmds.clear();
    m_PrimitiveIndexdDrawCmds.clear();
}

void GLRender::InitShaders() {
    Shader::InitGlobalBuffers();

    // Models

    m_ModelShader = new Shader();
    if ( !m_ModelShader->Load("shaders/entities.vert", "shaders/entities.frag", SHADER_FEATURE_MODEL_ANIMATION_BIT) ) {
        printf("Problems initializing model shaders!\n");
    }

    // Immediate mode Primitives: Lines, Tris, ...

    m_ImPrimitivesShader = new Shader();
    if ( !m_ImPrimitivesShader->Load("shaders/primitives.vert", "shaders/primitives.frag") ) {
        printf("Problems initializing primitives shader!\n");
    }

    // Colliders (for now: Ellipsoids, but can also be AABBs, etc.)

    m_ColliderShader = new Shader();
    if ( !m_ColliderShader->Load("shaders/colliders.vert", "shaders/colliders.frag") ) {
        printf("Problems initializin colliders shader!\n");
    }

    // TODO: Just to test if shaders overwrite data from each other. Delete later!
    Shader* foo = new Shader();
    if ( !foo->Load("shaders/entities.vert", "shaders/entities.frag", SHADER_FEATURE_MODEL_ANIMATION_BIT) ) {
        printf("Problems initializing model shaders!\n");
    }

    // 2D Screenspace: UI, Console, etc.

    m_FontShader = new Shader();
    if ( !m_FontShader->Load("shaders/font.vert", "shaders/font.frag") ) {
        printf("Problems initializing font shader!\n");
    }
    // TODO: We could think about actually creating a subclass
    //       for a dedicated screenspace shader so we don't have
    //       to do this weird call. It is easy to forget and
    //       also the main shader class has all these things related
    //       to 2d screenspace rendering which it doesn't need.
    //       But more things are to come so don't over-abstract things for now!
    m_FontShader->InitializeFontUniforms();

    m_ShapesShader = new Shader();
    if ( !m_ShapesShader->Load("shaders/shapes2d.vert", "shaders/shapes2d.frag") ) {
        printf("Problems initializing shapes2d shader!\n");
    }
    m_ShapesShader->InitializeShapesUniforms();

    m_CompositeShader = new Shader();
    if ( !m_CompositeShader->Load("shaders/composite.vert", "shaders/composite.frag") ) {
        printf("Problems initializing composite shader!\n");
    }

    m_WorldShader = new Shader();
    if ( !m_WorldShader->Load("shaders/world.vert", "shaders/world.frag") ) {
        printf("Problems initializing world shader!\n");
    }

    m_BrushShader = new Shader();
    if ( !m_BrushShader->Load("shaders/brush.vert", "shaders/brush.frag") ) {
        printf("Problems initializing brush shader!\n");
    }
}

void GLRender::SetWindowTitle(char* windowTitle) {
    SDL_SetWindowTitle(m_Window, windowTitle);
}
