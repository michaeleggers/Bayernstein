#include "r_gl.h"

#include <SDL.h>
#include <SDL_egl.h>
#include <glad/glad.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <SDL2/SDL_egl.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "r_common.h"
#include "platform.h"
#include "r_itexture.h"
#include "r_gl_batch.h"
#include "r_gl_texture.h"
#include "r_gl_texture_mgr.h"
#include "input.h" 

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

void GLRender::Shutdown(void)
{
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
    
    m_ModelShader->Unload();
    delete m_ModelShader;

    m_ImPrimitivesShader->Unload();
    delete m_ImPrimitivesShader;
}

bool GLRender::Init(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    // From 2.0.18: Enable native IME.
/*
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
*/

    // Create an application window with the following settings:
    m_Window = SDL_CreateWindow(
        "HKD",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

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
    if (!m_SDL_GL_Conext) {
        SDL_Log("Unable to create GL context! SDL-Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(m_Window, m_SDL_GL_Conext);

    int majorVersion;
    int minorVersion;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &majorVersion);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minorVersion);
    printf("OpenGL Version active: %d.%d\n", majorVersion, minorVersion);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to get OpenGL function pointers via GLAD: %s\n", SDL_GetError());
        return false;
    }

    printf("OpenGL Version active (via glGetString): %s\n", glGetString(GL_VERSION));

    // Check that the window was successfully created

    if (m_Window == NULL) {
        // In the case that the window could not be made...
        SDL_Log("Could not create window: %s\n", SDL_GetError());
        return false;
    }

    SDL_ShowWindow(m_Window);

    // GL Vsync on
    if (SDL_GL_SetSwapInterval(1) != 0) {
        SDL_Log("Failed to enable vsync!\n");
    }
    else {
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

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    SDL_Log("%s, %s\n", vendor, renderer);

    // Create batches

    // make space for 1Mio vertices
    // TODO: What should be the upper limit?
    // TODO: Shouldn't we just pass in the max size in bytes for GLBatch Ctor?
    // With sizeof(Vertex) = 92bytes => sizeof(Tri) = 276bytes we need ~ 263MB for Models.
    // A lot for a game in the 2000s! Our models have a tri count of maybe 3000 Tris (without weapon), which
    // is not even close to 1Mio tris.
    m_ModelBatch = new GLBatch(1000 * 1000);

    // Batches but for different purposes
    m_ImPrimitiveBatch = new GLBatch(1000 * 1000);
    m_ImPrimitiveBatchIndexed = new GLBatch(1000 * 1000, 1000 * 1000);
    m_ColliderBatch = new GLBatch(1000000);

    // Initialize shaders

    InitShaders();

    // Create and upload Collider Models to GPU

    RegisterColliderModels();

    return true;
}

// At the moment we don't generate a drawCmd for a model. We just but all of
// the meshes into a GPU buffer and draw all of them exactly the same way.
int GLRender::RegisterModel(HKD_Model* model)
{ 
    GLModel gl_model = {};
    int offset = m_ModelBatch->Add(&model->tris[0], model->tris.size());

    for (int i = 0; i < model->meshes.size(); i++) {
        HKD_Mesh* mesh = &model->meshes[i];
        GLTexture* texture = (GLTexture*)m_TextureManager->CreateTexture(mesh->textureFileName);        
        GLMesh gl_mesh = {
            .triOffset = offset/3 + (int)mesh->firstTri,
            .triCount = (int)mesh->numTris,
            .texture = texture
        };
        gl_model.meshes.push_back(gl_mesh);
    }

    m_Models.push_back(gl_model);

    int gpuModelHandle = m_Models.size() - 1;
    model->gpuModelHandle = gpuModelHandle;

    return gpuModelHandle;
}

void GLRender::SetActiveCamera(Camera* camera)
{
    m_ActiveCamera = camera;
}

void GLRender::RegisterColliderModels()
{
    // Generate vertices for a circle. Used for ellipsoid colliders.

    MeshEllipsoid unitEllipsoid = CreateUnitEllipsoid(2);

    m_EllipsoidColliderDrawCmd = AddTrisToBatch(
        m_ColliderBatch,
        unitEllipsoid.tris.data(), unitEllipsoid.tris.size(),
        false, DRAW_MODE_WIREFRAME);
}

// Maybe return a void* as GPU handle, because usually APIs that use the handle of
// a specific graphics API don't expect it to be in int or whatever.
std::vector<ITexture*> GLRender::ModelTextures(int gpuModelHandle)
{
    std::vector<ITexture*> results;
    
    if (gpuModelHandle >= m_Models.size()) return results;
    else if (gpuModelHandle < 0)           return results;

    GLModel* model = &m_Models[gpuModelHandle];
    for (auto& mesh : model->meshes) {
        results.push_back( mesh.texture );
    }

    return results;
}

std::vector<ITexture*> GLRender::Textures(void)
{
    std::vector<ITexture*> result;
    for (auto& elem: m_TextureManager->m_NameToTexture) {
        result.push_back(elem.second);
    }

    return result;
}

void GLRender::ImDrawTris(Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode)
{    
    int offset = m_ImPrimitiveBatch->Add(tris, numTris, cullFace, drawMode);       
    
    GLBatchDrawCmd drawCmd = {
        .offset = offset,
        .numVerts = 3 * numTris,
        .cullFace = cullFace,
        .drawMode = drawMode
    };

    m_PrimitiveDrawCmds.push_back(drawCmd);
}

void GLRender::ImDrawTriPlanes(TriPlane* triPlanes, uint32_t numTriPlanes, bool cullFace, DrawMode drawMode)
{
    std::vector<Tri> tris;
    tris.resize(numTriPlanes);
    for (int i = 0; i < numTriPlanes; i++) {
        tris[i] = triPlanes[i].tri;
    }
    ImDrawTris(tris.data(), numTriPlanes, cullFace, drawMode);
}

GLBatchDrawCmd GLRender::AddTrisToBatch(GLBatch* batch, Tri* tris, uint32_t numTris, bool cullFace, DrawMode drawMode) {
    int offset = batch->Add(tris, numTris, cullFace, drawMode);

    GLBatchDrawCmd drawCmd = {
        .offset = offset,
        .numVerts = 3 * numTris,
        .cullFace = cullFace,
        .drawMode = drawMode
    };

    return drawCmd;
}

// Draw triangles with indexed geometry :)
void GLRender::ImDrawIndexed(Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, bool cullFace, DrawMode drawMode)
{
    int offset = 0;
    int offsetIndices = 0;
    if (!m_ImPrimitiveBatchIndexed->Add(verts, numVerts, indices, numIndices, &offset, &offsetIndices, cullFace, drawMode)) {
        return;
    }

    GLBatchDrawCmd drawCmd = {        
        .offset = offset,
        .indexOffset = offsetIndices,
        .numVerts = numVerts,
        .numIndices = numIndices,
        .cullFace = cullFace,
        .drawMode = drawMode
    };

    m_PrimitiveIndexdDrawCmds.push_back(drawCmd);
}

// TODO: not done
void GLRender::ImDrawVerts(Vertex* verts, uint32_t numVerts) 
{
    int offset = m_ImPrimitiveBatch->Add(verts, numVerts);

    GLBatchDrawCmd drawCmd = {
        .offset = offset,
        .numVerts = numVerts,
        .cullFace = false,
        .drawMode = DRAW_MODE_SOLID
    };
}

// We have separate draw cmds from the batch. This function generate unneccessary many
// draw cmds (each Add is a new one!).
void GLRender::ImDrawLines(Vertex* verts, uint32_t numVerts, bool close)
{
    if (numVerts < 2) { // This won't work, man.
        return;
    }

    Vertex* v = verts;
    // TODO: DRAW_MODEL_LINES doesn't do anything to the batch!
    int offset = m_ImPrimitiveBatch->Add(v, 2, false, DRAW_MODE_LINES);
    v += 1;
    int moreVerts = 0;
    for (int i = 2; i < numVerts; i++) {
        m_ImPrimitiveBatch->Add(v, 2, false, DRAW_MODE_LINES);
        v++;        
        moreVerts += 1;
    }

    if (close) {
        Vertex endAndStart[] = { *v, verts[0] };
        m_ImPrimitiveBatch->Add(endAndStart, 2, false, DRAW_MODE_LINES);                
        moreVerts += 2;
    }

    GLBatchDrawCmd drawCmd = {
        .offset = offset,
        .numVerts = numVerts + moreVerts,
        .cullFace = false,
        .drawMode = DRAW_MODE_LINES
    };

    m_PrimitiveDrawCmds.push_back(drawCmd);
}

void GLRender::ImDrawSphere(glm::vec3 pos, float radius, glm::vec4 color)
{
    glm::mat4 view = m_ActiveCamera->ViewMatrix();
    // TODO: Global Setting for perspective values
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        (float)m_WindowWidth / (float)m_WindowHeight,
        0.1f, 10000.0f);

    m_ColliderShader->Activate();
    m_ColliderShader->SetViewProjMatrices(view, proj);

    m_ColliderBatch->Bind();
        m_ColliderShader->SetVec4("uDebugColor", color);
        glm::vec3 scale = glm::vec3(radius);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 M = T * S;
        m_ColliderShader->SetMat4("model", M);

        glDrawArrays(GL_TRIANGLES, m_EllipsoidColliderDrawCmd.offset, m_EllipsoidColliderDrawCmd.numVerts);
}

GLBatchDrawCmd GLRender::AddLineToBatch(GLBatch* batch, Vertex* verts, uint32_t numVerts, bool close)
{
    if (numVerts < 2) { // This won't work, man.
        return { -1 };
    }

    Vertex* v = verts;
    // TODO: DRAW_MODEL_LINES doesn't do anything to the batch!
    int offset = batch->Add(v, 2, false, DRAW_MODE_LINES);
    v += 1;
    int moreVerts = 0;
    for (int i = 2; i < numVerts; i++) {
        batch->Add(v, 2, false, DRAW_MODE_LINES);
        v++;
        moreVerts += 1;
    }

    if (close) {
        Vertex endAndStart[] = { *v, verts[0] };
        batch->Add(endAndStart, 2, false, DRAW_MODE_LINES);
        moreVerts += 2;
    }

    GLBatchDrawCmd drawCmd = {
        .offset = offset,
        .numVerts = numVerts + moreVerts,
        .cullFace = false,
        .drawMode = DRAW_MODE_LINES
    };

    return drawCmd;
}

void GLRender::RenderBegin(void)
{    
    // SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);
    
    // See: https://wiki.libsdl.org/SDL2/SDL_GL_GetDrawableSize
    SDL_GL_GetDrawableSize(m_Window, &m_WindowWidth, &m_WindowHeight);
    float windowAspect = (float)m_WindowWidth / (float)m_WindowHeight;
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
}

void GLRender::ExecuteDrawCmds(std::vector<GLBatchDrawCmd>& drawCmds, GeometryType geomType) 
{
    uint32_t prevDrawMode = GL_FILL;
    uint32_t primitiveType = GL_TRIANGLES;
    for (int i = 0; i < drawCmds.size(); i++) {

        GLBatchDrawCmd drawCmd = drawCmds[i];

        if (!drawCmd.cullFace) {
            glDisable(GL_CULL_FACE);
        }
        else {
            glEnable(GL_CULL_FACE);
        }

        if (prevDrawMode != drawCmd.drawMode) {
            if (drawCmd.drawMode == DRAW_MODE_WIREFRAME) {
                glLineWidth(1.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                m_ImPrimitivesShader->SetShaderSettingBits(SHADER_LINEMODE);
                prevDrawMode = GL_LINE;
                primitiveType = GL_TRIANGLES;
            }
            else if (drawCmd.drawMode == DRAW_MODE_LINES) {
                glLineWidth(5.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                m_ImPrimitivesShader->SetShaderSettingBits(SHADER_LINEMODE);
                prevDrawMode = GL_FILL;
                primitiveType = GL_LINES;
            }
            else { // DRAW_MODE_SOLID
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                prevDrawMode = GL_FILL;
                primitiveType = GL_TRIANGLES;
            }
        }
        
        if (geomType == GEOM_TYPE_VERTEX_ONLY) {
            glDrawArrays(primitiveType, drawCmd.offset, drawCmd.numVerts);
        }
        else if (geomType == GEOM_TYPE_INDEXED) {
            uint32_t indexBufferUSOffset = drawCmd.indexOffset * sizeof(uint16_t);
            glDrawElementsBaseVertex(primitiveType, drawCmd.numIndices, GL_UNSIGNED_SHORT,
                (GLvoid*)(drawCmd.indexOffset * sizeof(uint16_t)), drawCmd.offset);
        }

        m_ImPrimitivesShader->ResetShaderSettingBits(SHADER_LINEMODE);
    }
}

void GLRender::Render(Camera* camera, HKD_Model** models, uint32_t numModels)
{    
    // Camera and render settings

    static uint32_t drawWireframe = 0;

    if (KeyWentDown(SDLK_r)) { // WARNING: TAB key also triggers slider-values in ImGui Window.
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
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 10000.0f);

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


    // Draw Models

    m_ModelBatch->Bind();
    m_ModelShader->Activate();
    m_ModelShader->SetViewProjMatrices(view, proj);
    if (drawWireframe) {
        m_ModelShader->SetShaderSettingBits(SHADER_WIREFRAME_ON_MESH);
    }
    else {
        m_ModelShader->ResetShaderSettingBits(SHADER_WIREFRAME_ON_MESH);
    }    
    for (int i = 0; i < numModels; i++) {

        GLModel model = m_Models[ models[i]->gpuModelHandle ];

        if ( models[i]->numJoints > 0 ) {
            m_ModelShader->SetShaderSettingBits(SHADER_ANIMATED);
            m_ModelShader->SetMatrixPalette( &models[i]->palette[0], models[i]->numJoints );
        }
        else {
            m_ModelShader->ResetShaderSettingBits(SHADER_ANIMATED);
        }
        
        glm::mat4 modelMatrix = CreateModelMatrix( models[i] );
        m_ModelShader->SetMat4("model", modelMatrix);

        for (int j = 0; j < model.meshes.size(); j++) {
            GLMesh* mesh = &model.meshes[j];
            if (!mesh->texture->m_Filename.empty()) { // TODO: Checking string of empty is not great.
                m_ModelShader->SetShaderSettingBits(SHADER_IS_TEXTURED);
                glBindTexture(GL_TEXTURE_2D, mesh->texture->m_gl_Handle);
            }
            else {
                m_ModelShader->ResetShaderSettingBits(SHADER_IS_TEXTURED);
            }
            glDrawArrays(GL_TRIANGLES, 3*mesh->triOffset, 3 * mesh->triCount);
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

void GLRender::RenderColliders(Camera* camera, HKD_Model** models, uint32_t numModels)
{
    glm::mat4 view = camera->ViewMatrix();
    // TODO: Global Setting for perspective values
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        (float)m_WindowWidth / (float)m_WindowHeight,
        0.1f, 10000.0f);

    m_ColliderShader->Activate();
    m_ColliderShader->SetViewProjMatrices(view, proj);

    m_ColliderBatch->Bind();
    for (int i = 0; i < numModels; i++) {
        HKD_Model* model = models[i];
        m_ColliderShader->SetVec4("uDebugColor", model->debugColor);
        EllipsoidCollider ec = model->ellipsoidColliders[model->currentAnimIdx];
        glm::vec3 scale = glm::vec3(
            ec.radiusA,
            ec.radiusA,
            ec.radiusB);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), ec.center);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 M = T * S;
        m_ColliderShader->SetMat4("model", M);
        std::vector<GLBatchDrawCmd> drawCmds = {m_EllipsoidColliderDrawCmd};
        //glDrawArrays(GL_TRIANGLES, m_EllipsoidColliderDrawCmd.offset, m_EllipsoidColliderDrawCmd.numVerts);
        ExecuteDrawCmds(drawCmds, GEOM_TYPE_VERTEX_ONLY);
        // glDrawArrays(GL_LINES,
        //     m_EllipsoidColliderDrawCmd.offset,
        //     m_EllipsoidColliderDrawCmd.numVerts);
    }
}

void GLRender::RenderEnd(void)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(m_Window);

    m_ImPrimitiveBatch->Reset();
    m_ImPrimitiveBatchIndexed->Reset();
    m_PrimitiveDrawCmds.clear();
    m_PrimitiveIndexdDrawCmds.clear();
}

void GLRender::InitShaders()
{
    Shader::InitGlobalBuffers();

    // Models

    m_ModelShader = new Shader();
    if (!m_ModelShader->Load(
        "shaders/entities.vert",
        "shaders/entities.frag",
        SHADER_FEATURE_MODEL_ANIMATION_BIT
    )) {
        printf("Problems initializing model shaders!\n");
    }

    // Immediate mode Primitives: Lines, Tris, ...

    m_ImPrimitivesShader = new Shader();
    if (!m_ImPrimitivesShader->Load(
        "shaders/primitives.vert",
        "shaders/primitives.frag"
    )) {
        printf("Problems initializing primitives shader!\n");
    }

    // Colliders (for now: Ellipsoids, but can also be AABBs, etc.)

    m_ColliderShader = new Shader();
    if (!m_ColliderShader->Load(
        "shaders/colliders.vert",
        "shaders/colliders.frag"
        )) {
        printf("Problems initializin colliders shader!\n");
    }

    // TODO: Just to test if shaders overwrite data from each other. Delete later!
    Shader* foo = new Shader(); 
    if (!foo->Load(
        "shaders/entities.vert",
        "shaders/entities.frag",
        SHADER_FEATURE_MODEL_ANIMATION_BIT
    )) {
        printf("Problems initializing model shaders!\n");
    }
}

void GLRender::SetWindowTitle(char* windowTitle)
{
    SDL_SetWindowTitle(m_Window, windowTitle);
}

