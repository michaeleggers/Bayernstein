// NOTE: Header only libs *have* to be defined on top before everything else.
#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#include "game.h"

#include <SDL.h>
#include <string>

#include "imgui.h"

#include "./Message/message_type.h"
#include "CWorld.h"
#include "Path/path.h"
#include "Shape.h"
#include "ShapeSphere.h"
#include "camera.h"
#include "hkd_interface.h"
#include "input.h"
#include "physics.h"
#include "polysoup.h"
#include "r_font.h"
#include "r_itexture.h"
#include "utils/utils.h"
#include "input_handler.h"
#include "input_delegate.h"
#include "input_receiver.h"

static int hkd_Clamp(int val, int clamp) {
    if ( val > clamp || val < clamp ) return clamp;
    return val;
}

Game::Game(std::string exePath, hkdInterface* pInterface) {
    m_pInterface = pInterface;
    m_ExePath = exePath;
    m_pEntityManager = EntityManager::Instance();
}

void Game::Init() {
    m_AccumTime = 0.0f;

    // Load a font file from disk
    m_ConsoleFont = new CFont("fonts/HackNerdFont-Bold.ttf", 72);
    m_ConsoleFont30 = new CFont("fonts/HackNerdFont-Bold.ttf", 150); // Same font at different size

    IRender* renderer = GetRenderer();
    renderer->RegisterFont(m_ConsoleFont);
    renderer->RegisterFont(m_ConsoleFont30);

    // Load world triangles from Quake .MAP file

    //std::vector<MapTri> worldTris{};
    MapVersion mapVersion = VALVE_220; // TODO: Change to MAP_TYPE_QUAKE

    // TODO: Sane loading of Maps to be system independent ( see other resource loading ).
#ifdef _WIN32
    std::string mapData = loadTextFile(m_ExePath + "../../assets/maps/enemy_test.map");
#elif __LINUX__
    std::string mapData = loadTextFile(m_ExePath + "../assets/maps/temple2.map");
#endif

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[ 0 ], inputLength, mapVersion);

    m_World = CWorld::Instance();
    m_World->InitWorldFromMap(map);
    m_pPlayerEntity = m_World->PlayerEntity();

    // Register World Triangles at GPU.
    // Creates batches for each texture-name. That way we can
    // reduce draw-calls and texture-binds when rendering world geometry.

    renderer->RegisterWorld( m_World);

    //m_pPlayerEntity = new Player(idCounter++, m_pPlayerEntity->m_Position);
    //m_pEntityManager->RegisterEntity(m_pPlayerEntity);
 
#if 0 // Enable second debug player
    glm::vec3 dbgPlayerStartPos = m_pPlayerEntity->m_Position + glm::vec3(20.0f, -100.0f, 10.0f);
    m_pDebugPlayerEntity = new Player(dbgPlayerStartPos);
    printf("Debug Player Start Pos: %f, %f, %f\n", dbgPlayerStartPos.x, dbgPlayerStartPos.y, dbgPlayerStartPos.z);
    m_pEntityManager->RegisterEntity(m_pDebugPlayerEntity);
    int hDebugPlayerModel = renderer->RegisterModel(m_pDebugPlayerEntity->GetModel());
#endif 

    m_pFlyCameraEntity = new CFlyCamera( glm::vec3(0.0f) );
  
    // FIX: If the follow camera is registered *before* one of the entities
    // the follow camera will lag behind one frame because it won't
    // get the most up to date position of its target!
    // We can choose to have a dedicated array for all of the
    // camera entity types and let the entity manager take care
    // of correct update order.
    m_pFollowCameraEntity = new CFollowCamera(m_pPlayerEntity);
    m_pFollowCameraEntity->m_Camera = Camera(m_pPlayerEntity->m_Position);
    m_pFollowCameraEntity->m_Camera.m_Pos.y -= 200.0f;
    m_pFollowCameraEntity->m_Camera.m_Pos.z += 100.0f;
    m_pFollowCameraEntity->m_Camera.RotateAroundSide(0.0f);
    //m_pFollowCameraEntity->m_Camera.RotateAroundUp(180.0f);
    m_pEntityManager->RegisterEntity(m_pFollowCameraEntity);
   
    // Upload this model to the GPU. This will add the model to the model-batch and you get an ID where to find the data
    // in the batch?

    //int hPlayerModel = renderer->RegisterModel(m_pPlayerEntity->GetModel());
    //
   
    // Test Input Binding

    // Keyboard buttons
    CInputHandler* inputHandler = CInputHandler::Instance();
    inputHandler->BindInputToActionName(SDLK_SPACE, "jump");
    inputHandler->BindInputToActionName(SDLK_0, "equip_rocketlauncher");
    inputHandler->BindInputToActionName(SDLK_w, "forward");
    inputHandler->BindInputToActionName(SDLK_s, "back");
    inputHandler->BindInputToActionName(SDLK_a, "left");
    inputHandler->BindInputToActionName(SDLK_d, "right");
    inputHandler->BindInputToActionName(SDLK_LSHIFT, "speed");
    inputHandler->BindInputToActionName(SDLK_c, "set_captain");
    inputHandler->BindInputToActionName(SDLK_LEFT, "turn_left");
    inputHandler->BindInputToActionName(SDLK_RIGHT, "turn_right");
    // Mouse buttons
    inputHandler->BindInputToActionName(SDL_BUTTON_LEFT, "fire"); 
    inputHandler->BindInputToActionName(SDL_BUTTON_RIGHT, "look"); 

    // Let the player receive input by default
    CInputDelegate::Instance()->SetReceiver(m_pPlayerEntity);
}

static void DrawCoordinateSystem(IRender* renderer) {
    Vertex origin = { glm::vec3(0.0f) };
    origin.color = glm::vec4(1.0f);
    Vertex X = { glm::vec3(100.0f, 0.0f, 0.0f) };
    X.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    Vertex Y = { glm::vec3(0.0f, 100.0f, 0.0f) };
    Y.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    Vertex Z = { glm::vec3(0.0f, 0.0f, 100.0f) };
    Z.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    Vertex OX[] = { origin, X };
    Vertex OY[] = { origin, Y };
    Vertex OZ[] = { origin, Z };
    renderer->ImDrawLines(OX, 2);
    renderer->ImDrawLines(OY, 2);
    renderer->ImDrawLines(OZ, 2);
}

bool Game::RunFrame(double dt) {

    m_AccumTime += dt;

    // Want to quit on ESCAPE
    if ( KeyPressed(SDLK_ESCAPE) ) {
        m_pInterface->QuitGame();
    }

    // Toggle who should be controlled by the input system 
    static IInputReceiver* receivers[2] = { m_pPlayerEntity, m_pFlyCameraEntity };
    static BaseGameEntity* entities[2] = { m_pPlayerEntity, m_pFlyCameraEntity };
    static Camera* renderCam = &m_pFollowCameraEntity->m_Camera;

    // Toggle receivers
    static int receiverToggle = 0;
    if ( KeyWentDown(SDLK_u) ) {
        printf("Switching to receiver num: %d\n", receiverToggle);
        receiverToggle = ++receiverToggle % 2;
        CInputDelegate::Instance()->SetReceiver( receivers[ receiverToggle ] ); 
        m_pFollowCameraEntity->SetTarget( entities[ receiverToggle ] );
        renderCam = &m_pFollowCameraEntity->m_Camera;
        if ( entities[ receiverToggle ]->Type() == ET_FLY_CAMERA ) {
            CFlyCamera* flyCamEnt = (CFlyCamera*)entities[ receiverToggle ];
            renderCam = &flyCamEnt->m_Camera;
        }
    }
    // Handle the input
    CInputDelegate::Instance()->HandleInput();

    // Collide entities with the world geometry and bounce off of it.
    m_World->CollideEntitiesWithWorld();

    // Check if player has contacts with other entities (including brush entities such as doors).
    m_World->CollideEntities();


    // Run the message system
    m_pEntityManager->UpdateEntities();
    Dispatcher->DispatchDelayedMessages();


    // Fix camera position

    //m_pPlayerEntity->UpdateCamera(&m_FollowCamera);

    // Render stuff
    IRender* renderer = GetRenderer();

    // ImGUI stuff goes into GL default FBO

    ImGui::ShowDemoWindow();

    // Main 3D: This is where all the 3D rendering happens (in its own FBO)
    {
        renderer->Begin3D();

#if 0 // FIX: How to easily enable debug-line drawing?
        
        // Draw Debug Line for player veloctiy vector
        Line velocityDebugLine = { Vertex(ecEnemy.center), Vertex(ecEnemy.center + 150.0f * enemy->m_Velocity) };
        velocityDebugLine.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        velocityDebugLine.b.color = velocityDebugLine.a.color;
        renderer->ImDrawLines(velocityDebugLine.vertices, 2, false);
#endif
        // Render World Geometry (Batched triangles)
        //renderer->SetActiveCamera(&m_FollowCamera);
        //renderer->DrawWorldTris();

#if 0 // FIX: Give Brush entities pointers to their geometry data on GPU/CPU

        // Render Brush Entities
        for ( int i = 0; i < m_World.m_BrushEntities.size(); i++ ) {
            int be = m_World.m_BrushEntities[ i ];
            BaseGameEntity* pEntity = m_pEntityManager->GetEntityFromID(be);
            if ( pEntity->Type() == ET_DOOR ) {
                Door* pDoor = (Door*)pEntity;
                renderer->ImDrawMapTris(pDoor->Tris().data(),
                                          pDoor->Tris().size(),
                                          true,
                                          DRAW_MODE_SOLID);
            }
        }
#endif

        DrawCoordinateSystem(renderer);

#if 0 // debug path
        std::vector<Vertex> vertices = m_pPath->GetPointsAsVertices();
        renderer->ImDrawLines(vertices.data(), vertices.size(), true);
#endif

#if 1 // Toggle moving entity rendering. Also renders world.
        renderer->Render(renderCam, 
                         m_World->Models().data(), m_World->Models().size(),
                         m_World->BrushModels().data(), m_World->BrushModels().size());
#endif

        // auto type = enemy->m_Type;


#if 0
        if ( collisionInfo.didCollide ) {
            m_pPlayerEntity->GetModel()->debugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
        } else {
            m_pPlayerEntity->GetModel()->debugColor = glm::vec4(1.0f); // white
        }
#endif

#if 0 // Toggle draw hitpoint
        renderer->SetActiveCamera(&m_FollowCamera);
        renderer->ImDrawSphere(collisionInfo.hitPoint, 5.0f);
#endif

        //renderer->SetActiveCamera(&m_pFlyCameraEntity->m_Camera);
        HKD_Model* playerColliderModel[] = { m_pPlayerEntity->GetModel() };
        renderer->RenderColliders(renderCam, playerColliderModel, 1);
        
        renderer->End3D();
    } // End3D scope

#if 0 // Toggle 2D Font/Box renderingtest

    // Usage example of 2D Screenspace Rendering (useful for UI, HUD, Console...)
    // 2D stuff also has its own, dedicated FBO!
    {
        renderer->Begin2D(); // Enable screenspace 2D rendering. Binds the 2d offscreen framebuffer and activates the 2d shaders.
       
        //renderer->DrawBox( 10, 20, 200, 200, glm::vec4(0.4f, 0.3f, 1.0f, 1.0f) );
        renderer->SetFont( m_ConsoleFont, glm::vec4(0.0f, 0.0f, 0.0f, 0.75f) );
        renderer->R_DrawText("Welcome to the texture test.", 0.0f, 0.0f, COORD_MODE_ABS);
        
        renderer->SetFont( m_ConsoleFont, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) );
        renderer->R_DrawText("Welcome to the texture test.", 10.0f, 10.0f, COORD_MODE_ABS);
        
        // If you want to draw in absolute coordinates then you have to specify it.
        // Depends on the resolution of the render window! 
        renderer->SetFont( m_ConsoleFont, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) );
        renderer->R_DrawText("Some more text in yellow :)", 0.0f, 200.0f, COORD_MODE_ABS); 
        renderer->R_DrawText("And blended with box on top", 100.0f, 300.0f, COORD_MODE_ABS);

        renderer->SetShapeColor( glm::vec4(0.7f, 0.3f, 0.7f, 0.7) );
        renderer->DrawBox( 200.0f, 200.0f, 800.0f, 200.0f, COORD_MODE_ABS );

        renderer->SetShapeColor( glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) );
        renderer->DrawBox( 0.5f, 0.5f, 0.25f, 0.5f );

        renderer->SetFont( m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f) );

        // Use Relative coords (the default). Independent from screen resolution.
        // Goes from 0 (top/left) to 1 (bottom/right).
        renderer->R_DrawText("Waaay smaller text here!!!! (font size 30)", 0.5f, 0.5f); 

        renderer->SetShapeColor( glm::vec4(0.3f, 0.3f, 0.7f, 1.0) );
        renderer->DrawBox( 600, 600, 200, 100, COORD_MODE_ABS );
        renderer->SetFont( m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f) );
        renderer->R_DrawText("Waaay smaller text here!!!! (font size 30)", 
                             600.0f, 600.0f, COORD_MODE_ABS);
        renderer->SetFont( m_ConsoleFont30, glm::vec4(0.0f, 0.0f, 1.0f, 0.5f) );
        renderer->R_DrawText("Waaay smaller text here!!!! (font size 30)", 
                             605.0f, 605.0f, COORD_MODE_ABS);

        renderer->End2D(); // Stop 2D mode. Unbind 2d offscreen framebuffer.
    } // End2D Scope
#endif


    return true;
}

void Game::Shutdown() {

    // NOTE: m_pEntityManager is static memory and cannot deleted with delete
    // (it never was heap allocated with 'new'). The entities have to
    // be released manually.
    // delete m_pEntityManager;
    m_pEntityManager->KillEntities();
}

