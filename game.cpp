// NOTE: Header only libs *have* to be defined on top before everything else.
#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#include "game.h"

#include <SDL.h>
#include <string>

#include "imgui.h"

#include "CWorld.h"
#include "Shape.h"
#include "ShapeSphere.h"
#include "camera.h"
#include "input.h"
#include "physics.h"
#include "r_itexture.h"
#include "utils.h"
#include "./Message/message_type.h"
#include "r_font.h"
#include "polysoup.h"
#include "hkd_interface.h"

static int hkd_Clamp(int val, int clamp) {
    if (val > clamp || val < clamp) return clamp;
    return val;
}

Game::Game(std::string exePath, hkdInterface* interface) {
    m_Interface = interface;
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

    std::vector<MapTri> worldTris{};
    MapVersion mapVersion = VALVE_220; // TODO: Change to MAP_TYPE_QUAKE
    
    // TODO: Sane loading of Maps to be system independent ( see other resource loading ).
#ifdef _WIN32
    std::string mapData = loadTextFile(m_ExePath + "../../assets/maps/temple2.map");
#elif __LINUX__
    std::string mapData = loadTextFile(m_ExePath + "../assets/maps/temple2.map");
#endif

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[0], inputLength, mapVersion); 
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    std::vector<MapPolygon> tris = triangulate(polysoup);
    
    glm::vec4 triColor = glm::vec4( 0.1f, 0.8f, 1.0f, 1.0f );
    for (int i = 0; i < tris.size(); i++) {
        MapPolygon mapPoly = tris[ i ];
      
        Vertex A = { 
            glm::vec3(mapPoly.vertices[0].pos.x,
                      mapPoly.vertices[0].pos.y,
                      mapPoly.vertices[0].pos.z),
                      mapPoly.vertices[0].uv };
        Vertex B = { 
            glm::vec3(mapPoly.vertices[1].pos.x,
                      mapPoly.vertices[1].pos.y,
                      mapPoly.vertices[1].pos.z),
                      mapPoly.vertices[1].uv };
        Vertex C = { 
            glm::vec3(mapPoly.vertices[2].pos.x,
                      mapPoly.vertices[2].pos.y,
                      mapPoly.vertices[2].pos.z),
                      mapPoly.vertices[2].uv };

        A.color = triColor;
        B.color = triColor;
        C.color = triColor;
        MapTri tri = { .tri = {A, B, C} };
        tri.textureName = mapPoly.textureName;
        //FIX: Search through all supported image formats not just PNG.
        tri.hTexture = renderer->RegisterTextureGetHandle(tri.textureName + ".tga");
        worldTris.push_back(tri);
    }
   

    m_World.InitWorld(
        worldTris,
        glm::vec3(0.0f, 0.0f, -0.5f)); // gravity

    int idCounter = 0; //FIX: Responsibility of entity manager. Move to InitWorld.
    // Load and create all the entities
    for (int i = 0; i < map.entities.size(); i++) {
        Entity& e = map.entities[ i ];
        BaseGameEntity* baseEntity = NULL;
        // Check the classname
        for (int j = 0; j < e.properties.size(); j++) {
            Property& prop = e.properties[ j ];
            if (prop.key == "classname") {
                if (prop.value == "func_door") {
                    baseEntity = new Door( idCounter++, e.properties, e.brushes ); // later: Entity manager allocates entities!
                    m_World.m_BrushEntities.push_back( baseEntity->ID() );
                    m_pEntityManager->RegisterEntity(baseEntity);
                }
                else {
                    printf("Unknown entity type: %s\n", prop.value.c_str());
                }
            }
        }
    }
    // Register World Triangles at GPU.
    // Creates batches for each texture-name. That way we can
    // reduce draw-calls and texture-binds when rendering world geometry.

    renderer->RegisterWorldTris( m_World.m_MapTris );
    

    // Load IQM Model

    IQMModel iqmModel = LoadIQM("models/multiple_anims/multiple_anims.iqm");

    // Convert the model to our internal format

    // m_Model2 = CreateModelFromIQM(&iqmModel2, nullptr);
    m_Player = CreateModelFromIQM(&iqmModel);
    m_Player.isRigidBody = false;
    m_Player.position = glm::vec3(-96, -192, 88);
    m_Player.scale = glm::vec3(22.0f);
    for (int i = 0; i < m_Player.animations.size(); i++) {
        EllipsoidCollider* ec = &m_Player.ellipsoidColliders[i];
        ec->radiusA *= m_Player.scale.x;
        ec->radiusB *= m_Player.scale.z;
        ec->center = m_Player.position + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace = glm::scale(glm::mat4(1.0f), scale);
    }

    SetAnimState(&m_Player, ANIM_STATE_WALK);

    // Upload this model to the GPU. This will add the model to the model-batch and you get an ID where to find the data
    // in the batch?

    int hPlayerModel = renderer->RegisterModel(&m_Player);

    // Cameras

    m_Camera = Camera(glm::vec3(0, -1000.0f, 600.0));
    m_Camera.RotateAroundSide(-15.0f);

    m_FollowCamera = Camera(m_Player.position);
    m_FollowCamera.m_Pos.y -= 200.0f;
    m_FollowCamera.m_Pos.z += 100.0f;
    m_FollowCamera.RotateAroundSide(0.0f);
    m_FollowCamera.RotateAroundUp(180.0f);


    m_pPlayerEntity = new Player( idCounter++ );
    m_pEntityManager->RegisterEntity(m_pPlayerEntity);
    m_pEntityManager->RegisterEntity(new Enemy( idCounter++ ));
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

    if (KeyPressed(SDLK_ESCAPE)) {
        m_Interface->QuitGame();
    }

    float followCamSpeed = 0.5f;
    float followTurnSpeed = 0.2f;
    if (KeyPressed(SDLK_LSHIFT)) {
        followCamSpeed *= 0.1f;
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
    glm::vec3 forward = glm::rotate(
        orientation, glm::vec3(0.0f, -1.0f, 0.0f)); // -1 because the model is facing -1 (Outside the screen)
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


    EllipsoidCollider ec = m_Player.ellipsoidColliders[m_Player.currentAnimIdx];
   
    // Test collision between player and world geometry
#if 1
    // FIX: Just for the record. This is super dumb and not performant at all!
    // I (Michael) just did this to test the messaging system with doors.
    // We should have a list of brush entity geometry that is attached
    // to the list of static world geometry. The renderer allocates two
    // buffers: One GPU only buffer for static geometry.
    // One GPU/CPU buffer for brush entities that we have to update.
    // But on CPU side all of the tries should reside in ONE buffer as
    // this guarantees cache locality and a *MUCH* easier time for the
    // collision system!!! Arrays just always win... what can I say?
    // The brush entities should have pointers (indices) into the
    // CPU-side triangle array to know what geometry belongs to them.
    std::vector<MapTri> allTris = m_World.m_MapTris;
    int be = m_World.m_BrushEntities[ 0 ];
    Door* pEntity = (Door*)m_pEntityManager->GetEntityFromID( be );
    std::copy( pEntity->MapTris().begin(), pEntity->MapTris().end(), std::back_inserter(allTris) ); 
    
    CollisionInfo collisionInfo = CollideEllipsoidWithMapTris(ec,
                                                               static_cast<float>(dt) * m_Player.velocity,
                                                               static_cast<float>(dt) * m_World.m_Gravity,
                                                               allTris.data(),
                                                               allTris.size());

    // Update the ellipsoid colliders for all animation states based on the new collision position
    for (int i = 0; i < m_Player.animations.size(); i++) {
        m_Player.ellipsoidColliders[i].center = collisionInfo.basePos;
    }
    m_Player.position.x = collisionInfo.basePos.x;
    m_Player.position.y = collisionInfo.basePos.y;
    m_Player.position.z = collisionInfo.basePos.z - ec.radiusB;
#endif

    
    // Check if player runs against door
#if 1 
    for (int i = 0; i < m_World.m_BrushEntities.size(); i++) { 
        int be = m_World.m_BrushEntities[ i ];
        BaseGameEntity* pEntity = m_pEntityManager->GetEntityFromID( be );
        if ( pEntity->Type() == ET_DOOR ) {
            Door* pDoor = (Door*)pEntity;
            CollisionInfo ciDoor = PushTouch(ec,
                                             static_cast<float>(dt) * m_Player.velocity, 
                                             pDoor->MapTris().data(), 
                                             pDoor->MapTris().size() );
            if (ciDoor.didCollide) { 
                printf("COLLIDED!\n");
                Dispatcher->DispatchMessage(SEND_MSG_IMMEDIATELY,
                                            m_pPlayerEntity->ID(), pDoor->ID(), 
                                            message_type::Collision, 
                                            0); 
            }
        }
    }
#endif

    UpdateModel(&m_Player, (float)dt);

    // Run the message system
    m_pEntityManager->UpdateEntities();
    Dispatcher->DispatchDelayedMessages();
    

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
    IRender* renderer = GetRenderer();
    // ImGUI stuff goes into GL default FBO
    renderer->RenderBegin();

    ImGui::ShowDemoWindow();

    // Main 3D: This is where all the 3D rendering happens (in its own FBO)
    {
        renderer->Begin3D();
        
        // Draw Debug Line for player veloctiy vector
        Line velocityDebugLine = {
            Vertex(ec.center), Vertex(ec.center + 200.0f * m_Player.velocity)
        };
        velocityDebugLine.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        velocityDebugLine.b.color = velocityDebugLine.a.color;
        renderer->ImDrawLines(velocityDebugLine.vertices, 2, false);

        // Render World Geometry (Batched triangles)
        //renderer->SetActiveCamera(&m_FollowCamera);
        //renderer->DrawWorldTris();

#if 0
        // Render World geometry
        renderer->ImDrawMapTris(m_World.m_Tris.data(), 
                                  m_World.m_Tris.size(), 
                                  true,
                                  DRAW_MODE_SOLID);

        // Render Brush Entities
        for (int i = 0; i < m_World.m_BrushEntities.size(); i++) {
            int be = m_World.m_BrushEntities[ i ];
            BaseGameEntity* pEntity = m_pEntityManager->GetEntityFromID( be );
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

        HKD_Model* renderModels[1];
        renderModels[0] = &m_Player;

        renderer->Render(
            &m_FollowCamera,
            renderModels, 1);


        if (collisionInfo.didCollide) {
            m_Player.debugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
        } else {
            m_Player.debugColor = glm::vec4(1.0f); // white
        }

#if 0 // Toggle draw hitpoint
        renderer->SetActiveCamera(&m_FollowCamera);
        renderer->ImDrawSphere(collisionInfo.hitPoint, 5.0f);
#endif

        // Render Player's ellipsoid collider
        renderer->SetActiveCamera(&m_FollowCamera);
        HKD_Model* playerColliderModel[] = { &m_Player };
        renderer->RenderColliders(&m_FollowCamera, playerColliderModel, 1);
        
        renderer->End3D();

    } // End3D scope
   

#if 1 // Toggle 2D Font/Box renderingtest
    
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

    // This call composits 2D and 3D together into the default FBO
    // (along with ImGUI).
    renderer->RenderEnd(); 

    return true;
}

void Game::Shutdown() {

    // NOTE: m_pEntityManager is static memory and cannot deleted with delete
    // (it never was heap allocated with 'new'). The entities have to
    // be released manually.
    // delete m_pEntityManager;
    m_pEntityManager->KillEntities();
}

