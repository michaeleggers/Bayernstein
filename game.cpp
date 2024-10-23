#include "game.h"

#include <SDL.h>
#include <string>

#include "r_itexture.h"
#include "camera.h"
#include "input.h" 
#include "physics.h"
#include "utils.h"
#include "Shape.h"
#include "ShapeSphere.h"
#include "CWorld.h"
#include "r_font.h"
#include "imgui.h"


#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#include "polysoup.h"

static int hkd_Clamp(int val, int clamp) {
    if (val > clamp || val < clamp) return clamp;
    return val;
}

Game::Game(std::string exePath, hkdInterface* interface, IRender* renderer)
{
    m_Renderer = renderer;
    m_Interface = interface;
    m_ExePath = exePath;
}

void Game::Init()
{
    m_AccumTime = 0.0f;

    // Load a font file from disk
    m_ConsoleFont = new CFont("fonts/HackNerdFont-Bold.ttf", 72);
    m_ConsoleFont30 = new CFont("fonts/HackNerdFont-Bold.ttf", 150); // Same font at different size
    m_Renderer->RegisterFont(m_ConsoleFont);
    m_Renderer->RegisterFont(m_ConsoleFont30);
    
    // Load world triangles from Quake .MAP file

    std::vector<TriPlane> worldTris{};
    MapVersion mapVersion = VALVE_220; // TODO: Change to MAP_TYPE_QUAKE
    
    // TODO: Sane loading of Maps to be system independent ( see other resource loading ).
#ifdef _WIN32
    std::string mapData = loadTextFile(m_ExePath + "../../assets/maps/room.map"); 
#elif __LINUX__
    std::string mapData = loadTextFile(m_ExePath + "../assets/maps/room.map"); 
#endif

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[0], inputLength, mapVersion); 
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    std::vector<MapPolygon> tris = triangulate(polysoup);
    
    glm::vec4 triColor = glm::vec4( 0.1f, 0.8f, 1.0f, 1.0f );
    for (int i = 0; i < tris.size(); i++) {
        MapPolygon mapPoly = tris[ i ];
        Vertex A = { glm::vec3(mapPoly.vertices[0].x, mapPoly.vertices[0].y, mapPoly.vertices[0].z) };
        Vertex B = { glm::vec3(mapPoly.vertices[1].x, mapPoly.vertices[1].y, mapPoly.vertices[1].z) };
        Vertex C = { glm::vec3(mapPoly.vertices[2].x, mapPoly.vertices[2].y, mapPoly.vertices[2].z) };
        A.color = triColor;
        B.color = triColor;
        C.color = triColor;
        Tri tri = { A, B, C };

        TriPlane triPlane{};
        triPlane.tri = tri;
        triPlane.plane = CreatePlaneFromTri(triPlane.tri);
        triPlane.tri.a.normal = triPlane.plane.normal;
        triPlane.tri.b.normal = triPlane.plane.normal;
        triPlane.tri.c.normal = triPlane.plane.normal;
        worldTris.push_back( triPlane );    
    }
    m_World.InitWorld( worldTris.data(), worldTris.size(), glm::vec3(0.0f, 0.0f, -0.5f) );

    // Load IQM Model

    IQMModel iqmModel  = LoadIQM("models/multiple_anims/multiple_anims.iqm");

    // Convert the model to our internal format

    //m_Model2 = CreateModelFromIQM(&iqmModel2, nullptr);
    m_Player = CreateModelFromIQM(&iqmModel);
    m_Player.isRigidBody = false;
    m_Player.position = glm::vec3(-48.0f, 352.0f, 48.0f);
    m_Player.scale = glm::vec3(22.0f);
    for (int i = 0; i < m_Player.animations.size(); i++) {
        EllipsoidCollider* ec = &m_Player.ellipsoidColliders[i];
        ec->radiusA *= m_Player.scale.x;
        ec->radiusB *= m_Player.scale.z;
        ec->center = m_Player.position + glm::vec3(
            0.0f,
            0.0f,
            ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
    }

    SetAnimState(&m_Player, ANIM_STATE_WALK);

    // Upload this model to the GPU. This will add the model to the model-batch and you get an ID where to find the data in the batch?

    int hPlayerModel = m_Renderer->RegisterModel(&m_Player);

    // Cameras

    m_Camera = Camera(glm::vec3(0, -1000.0f, 600.0));
    m_Camera.RotateAroundSide(-15.0f);

    m_FollowCamera = Camera(m_Player.position);
    m_FollowCamera.m_Pos.y -= 200.0f;
    m_FollowCamera.m_Pos.z += 100.0f;
    m_FollowCamera.RotateAroundSide(-20.0f);    
    m_FollowCamera.RotateAroundUp(180.0f);
}

static void DrawCoordinateSystem(IRender * renderer) {
    Vertex origin = {glm::vec3(0.0f)};
    origin.color = glm::vec4(1.0f);
    Vertex X = {glm::vec3(100.0f, 0.0f, 0.0f)};
    X.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    Vertex Y = {glm::vec3(0.0f, 100.0f, 0.0f)};
    Y.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    Vertex Z = {glm::vec3(0.0f, 0.0f, 100.0f)};
    Z.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    Vertex OX[] = {origin, X};
    Vertex OY[] = {origin, Y};
    Vertex OZ[] = {origin, Z};
    renderer->ImDrawLines(OX, 2);
    renderer->ImDrawLines(OY, 2);
    renderer->ImDrawLines(OZ, 2);
}

bool Game::RunFrame(double dt)
{    
    m_AccumTime += dt;


    // Want to quit on ESCAPE

    if (KeyPressed(SDLK_ESCAPE)) {
        m_Interface->QuitGame();
    }


    float followCamSpeed = 0.5f;
    float followTurnSpeed = 0.2f;
    if (KeyPressed(SDLK_LSHIFT)) {
        followCamSpeed *= 0.3f;
        followTurnSpeed *= 0.3f;
    }

    // Model rotation

    float r = followTurnSpeed * dt;
    if (KeyPressed(SDLK_RIGHT)) {
        glm::quat rot = glm::angleAxis(glm::radians(-r), glm::vec3(0.0f, 0.0f, 1.0f));
        m_Player.orientation *= rot;
        m_FollowCamera.RotateAroundUp(-r);
    }
    if (KeyPressed(SDLK_LEFT)) {
        glm::quat rot = glm::angleAxis(glm::radians(r), glm::vec3(0.0f, 0.0f, 1.0f));
        m_Player.orientation *= rot;
        m_FollowCamera.RotateAroundUp(r);
    }
    glm::quat orientation = m_Player.orientation;
    glm::vec3 forward = glm::rotate(orientation, glm::vec3(0.0f, -1.0f, 0.0f)); // -1 because the model is facing -1 (Outside the screen)
    glm::vec3 side = glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f));

    // Change player's velocity and animation state based on input

    m_Player.velocity = glm::vec3(0.0f);
    float t = followCamSpeed;
    AnimState playerAnimState = ANIM_STATE_IDLE;
    if (KeyPressed(SDLK_w)) {          
        m_Player.velocity += t * forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if (KeyPressed(SDLK_s)) {        
        m_Player.velocity -= t * forward;
        playerAnimState = ANIM_STATE_RUN;
    }
    if (KeyPressed(SDLK_d)) {
        m_Player.velocity += t * side;
        playerAnimState = ANIM_STATE_RUN;        
    }
    if (KeyPressed(SDLK_a)) {
        m_Player.velocity -= t * side;
        playerAnimState = ANIM_STATE_RUN;
    }

    if (playerAnimState == ANIM_STATE_RUN) {
        if (KeyPressed(SDLK_LSHIFT)) {
            playerAnimState = ANIM_STATE_WALK;
        }
    }

    SetAnimState(&m_Player, playerAnimState);


    // Test collision between player and world geometry
    EllipsoidCollider ec = m_Player.ellipsoidColliders[m_Player.currentAnimIdx];
    CollisionInfo collisionInfo = CollideEllipsoidWithTriPlane(ec,
                                                               static_cast<float>(dt)*m_Player.velocity, 
                                                               static_cast<float>(dt)*m_World.m_Gravity, 
                                                               m_World.m_TriPlanes.data(), m_World.m_TriPlanes.size());

    // Update the ellipsoid colliders for all animation states based on the new collision position
    for (int i = 0; i < m_Player.animations.size(); i++) {
        m_Player.ellipsoidColliders[ i ].center = collisionInfo.basePos;
    }
    m_Player.position.x = collisionInfo.basePos.x;
    m_Player.position.y = collisionInfo.basePos.y;
    m_Player.position.z = collisionInfo.basePos.z - ec.radiusB;

    UpdateModel(&m_Player, (float)dt);
    
    // Fix camera position

    m_FollowCamera.m_Pos.x = m_Player.position.x;
    m_FollowCamera.m_Pos.y = m_Player.position.y;
    m_FollowCamera.m_Pos.z = m_Player.position.z + 70.0f;
    m_FollowCamera.m_Pos += (-forward * 80.0f);

    // Update camera
    float camSpeed = 0.5f;
    float turnSpeed = 0.2f;
    if (KeyPressed(SDLK_LSHIFT)) {
        camSpeed *= 0.1f;
        turnSpeed *= 0.25f;
    }
    if (KeyPressed(SDLK_w)) {
        m_Camera.Pan((float)dt * camSpeed * m_Camera.m_Forward);
    }
    if (KeyPressed(SDLK_s)) {
        m_Camera.Pan((float)dt * camSpeed * -m_Camera.m_Forward);
    }
    if (KeyPressed(SDLK_d)) {
        m_Camera.Pan((float)dt * camSpeed * m_Camera.m_Side);
    }
    if (KeyPressed(SDLK_a)) {
        m_Camera.Pan((float)dt * camSpeed * -m_Camera.m_Side);
    }

    if (KeyPressed(SDLK_RIGHT)) {
        m_Camera.RotateAroundUp(-turnSpeed * dt);
    }
    if (KeyPressed(SDLK_LEFT)) {
        m_Camera.RotateAroundUp(turnSpeed * dt);
    }

    if (KeyPressed(SDLK_UP)) {
        m_Camera.RotateAroundSide(turnSpeed * dt);
    }
    if (KeyPressed(SDLK_DOWN)) {
        m_Camera.RotateAroundSide(-turnSpeed * dt);
    }

    // Render stuff
    
    // ImGUI stuff goes into GL default FBO
    m_Renderer->RenderBegin();

    ImGui::ShowDemoWindow();

    // Main 3D: This is where all the 3D rendering happens (in its own FBO)
    {
        m_Renderer->Begin3D();
        
        // Draw Debug Line for player veloctiy vector
        Line velocityDebugLine = {
            Vertex(ec.center), Vertex(ec.center + 200.0f * m_Player.velocity)
        };
        velocityDebugLine.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        velocityDebugLine.b.color = velocityDebugLine.a.color;
        m_Renderer->ImDrawLines(velocityDebugLine.vertices, 2, false);


        // Render World geometry
        m_Renderer->ImDrawTriPlanes(m_World.m_TriPlanes.data(), m_World.m_TriPlanes.size(),
            true, DRAW_MODE_SOLID);


        DrawCoordinateSystem(m_Renderer);

        HKD_Model* renderModels[1];
        renderModels[0] = &m_Player;

        m_Renderer->Render(
            &m_FollowCamera,
            renderModels, 1);

        if (collisionInfo.didCollide) {
            m_Player.debugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
            m_Renderer->SetActiveCamera(&m_FollowCamera);
            m_Renderer->ImDrawSphere(collisionInfo.hitPoint, 5.0f);
                    //printf("hitpoint: %f, %f, %f\n", collisionInfo.hitPoint.x, collisionInfo.hitPoint.y, collisionInfo.hitPoint.z);
        } else {
            m_Player.debugColor = glm::vec4(1.0f); // white
        }

            // Render Player's ellipsoid collider
        m_Renderer->SetActiveCamera(&m_FollowCamera);
        HKD_Model* playerColliderModel[] = {&m_Player};
        m_Renderer->RenderColliders(&m_FollowCamera, playerColliderModel, 1);

        m_Renderer->End3D();
    }

    #if 1 // Toggle 2D Font/Box renderingtest
    
    // Usage example of 2D Screenspace Rendering (useful for UI, HUD, Console...)
    // 2D stuff also has its own, dedicated FBO!
    {
        m_Renderer->Begin2D(); // Enable screenspace 2D rendering. Binds the 2d offscreen framebuffer and activates the 2d shaders.
       
        //m_Renderer->DrawBox( 10, 20, 200, 200, glm::vec4(0.4f, 0.3f, 1.0f, 1.0f) );
        m_Renderer->SetFont( m_ConsoleFont, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) );
        m_Renderer->DrawText("ABCDEFGHIJKLMNOajdidjST*~`!/]}]|!#@#=;'\"$%%^&*():L", 0.0f, 0.0f);
        
        // If you want to draw in absolute coordinates then you have to specify it.
        // Depends on the resolution of the render window! 
        m_Renderer->SetFont( m_ConsoleFont, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) );
        m_Renderer->DrawText("Some more text in yellow :)", 0.0f, 200.0f, COORD_MODE_ABS); 
        m_Renderer->DrawText("And blended with box on top", 100.0f, 300.0f, COORD_MODE_ABS);

        m_Renderer->SetShapeColor( glm::vec4(0.7f, 0.3f, 0.7f, 0.7) );
        m_Renderer->DrawBox( 200.0f, 200.0f, 800.0f, 200.0f, COORD_MODE_ABS );

        m_Renderer->SetShapeColor( glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) );
        m_Renderer->DrawBox( 0.5f, 0.5f, 0.25f, 0.5f );

        m_Renderer->SetFont( m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f) );

        // Use Relative coords (the default). Independent from screen resolution.
        // Goes from 0 (top/left) to 1 (bottom/right).
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 0.5f, 0.5f); 

        m_Renderer->SetShapeColor( glm::vec4(0.3f, 0.3f, 0.7f, 1.0) );
        m_Renderer->DrawBox( 600, 600, 200, 100, COORD_MODE_ABS );
        m_Renderer->SetFont( m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f) );
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 
                             600.0f, 600.0f, COORD_MODE_ABS);
        m_Renderer->SetFont( m_ConsoleFont30, glm::vec4(0.0f, 0.0f, 1.0f, 0.5f) );
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 
                             605.0f, 605.0f, COORD_MODE_ABS);

        m_Renderer->End2D(); // Stop 2D mode. Unbind 2d offscreen framebuffer.
    } 
    #endif

    // This call composits 2D and 3D together into the default FBO
    // (along with ImGUI).
    m_Renderer->RenderEnd(); 


    return true;
}

void Game::Shutdown()
{
    m_Renderer->Shutdown();
    delete m_Renderer;
}
