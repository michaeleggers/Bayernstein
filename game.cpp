// NOTE: Header only libs *have* to be defined on top before everything else.
#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#include "game.h"

#include <SDL.h>
#include <string>

#include "imgui.h"

#include "./Message/message_type.h"
#include "CWorld.h"
#include "Shape.h"
#include "ShapeSphere.h"
#include "camera.h"
#include "input.h"
#include "physics.h"
#include "polysoup.h"
#include "r_font.h"
#include "r_itexture.h"
#include "utils/utils.h"

static int hkd_Clamp(int val, int clamp) {
    if ( val > clamp || val < clamp ) return clamp;
    return val;
}

Game::Game(std::string exePath, hkdInterface* interface, IRender* renderer) {
    m_Renderer = renderer;
    m_Interface = interface;
    m_ExePath = exePath;
    m_pEntityManager = EntityManager::Instance();
}

void Game::Init() {
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
    std::string mapData = loadTextFile(m_ExePath + "../assets/maps/enemy_test.map");
#endif

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[ 0 ], inputLength, mapVersion);
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    std::vector<MapPolygon> tris = triangulate(polysoup);

    glm::vec4 triColor = glm::vec4(0.1f, 0.8f, 1.0f, 1.0f);
    for ( int i = 0; i < tris.size(); i++ ) {
        MapPolygon mapPoly = tris[ i ];

        Vertex A = { glm::vec3(mapPoly.vertices[ 0 ].x, mapPoly.vertices[ 0 ].y, mapPoly.vertices[ 0 ].z) };
        Vertex B = { glm::vec3(mapPoly.vertices[ 1 ].x, mapPoly.vertices[ 1 ].y, mapPoly.vertices[ 1 ].z) };
        Vertex C = { glm::vec3(mapPoly.vertices[ 2 ].x, mapPoly.vertices[ 2 ].y, mapPoly.vertices[ 2 ].z) };
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

        worldTris.push_back(triPlane);
    }

    m_World.InitWorld(worldTris.data(), worldTris.size(), glm::vec3(0.0f, 0.0f, -0.5f)); // gravity

    int idCounter = 0; //FIX: Responsibility of entity manager
    glm::vec3 playerStartPosition = glm::vec3(0.0f);
    // Load and create all the entities
    for ( int i = 0; i < map.entities.size(); i++ ) {
        Entity& e = map.entities[ i ];
        BaseGameEntity* baseEntity = NULL;
        // Check the classname
        for ( int j = 0; j < e.properties.size(); j++ ) {
            Property& prop = e.properties[ j ];
            if ( prop.key == "classname" ) {
                if ( prop.value == "func_door" ) {
                    baseEntity
                        = new Door(idCounter++, e.properties, e.brushes); // later: Entity manager allocates entities!
                    m_World.m_BrushEntities.push_back(baseEntity->ID());
                    m_pEntityManager->RegisterEntity(baseEntity);
                } else if ( prop.value == "info_player_start" ) {
                    for ( Property& property : e.properties ) {
                        if ( property.key == "origin" ) {
                            std::vector<float> values = ParseFloatValues(property.value);
                            playerStartPosition = glm::vec3(values[ 0 ], values[ 1 ], values[ 2 ]);
                        }
                    }

                } else if ( prop.value == "" ) {
                } else {
                    printf("Unknown entity type: %s\n", prop.value.c_str());
                }
            }
        }
    }

    // Cameras
    m_FollowCamera = Camera(playerStartPosition);
    m_FollowCamera.m_Pos.y -= 200.0f;
    m_FollowCamera.m_Pos.z += 100.0f;
    m_FollowCamera.RotateAroundSide(-20.0f);
    m_FollowCamera.RotateAroundUp(180.0f);

    m_pPlayerEntity = new Player(idCounter++, playerStartPosition);
    m_pEntityManager->RegisterEntity(m_pPlayerEntity);

    Enemy* enemy = new Enemy(idCounter++);
    m_pEntityManager->RegisterEntity(enemy);

    // Upload this model to the GPU. This will add the model to the model-batch and you get an ID where to find the data
    // in the batch?

    int hPlayerModel = m_Renderer->RegisterModel(m_pPlayerEntity->GetModel());
    int hEnemyModel = m_Renderer->RegisterModel(enemy->GetModel());
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
        m_Interface->QuitGame();
    }

    EllipsoidCollider ec = m_pPlayerEntity->GetEllipsoidCollider();

    Enemy* enemy = m_pEntityManager->GetFirstEnemy();

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
    std::vector<TriPlane> allTris = m_World.m_TriPlanes;
#if 0 // if there are no doors in the world, this is not needed
    int be = m_World.m_BrushEntities[ 0 ];
    Door* pEntity = (Door*)m_pEntityManager->GetEntityFromID(be);
    std::copy(pEntity->TriPlanes().begin(), pEntity->TriPlanes().end(), std::back_inserter(allTris));
#endif

    CollisionInfo collisionInfo = CollideEllipsoidWithTriPlane(ec,
                                                               static_cast<float>(dt) * m_pPlayerEntity->GetVelocity(),
                                                               static_cast<float>(dt) * m_World.m_Gravity,
                                                               allTris.data(),
                                                               allTris.size());

    m_pPlayerEntity->UpdatePosition(collisionInfo.basePos);

    EllipsoidCollider ecEnemy = enemy->GetEllipsoidCollider();
    CollisionInfo enemyCollisionInfo = CollideEllipsoidWithTriPlane(ecEnemy,
                                                                    static_cast<float>(dt) * enemy->GetVelocity(),
                                                                    static_cast<float>(dt) * m_World.m_Gravity,
                                                                    allTris.data(),
                                                                    allTris.size());

    enemy->UpdatePosition(enemyCollisionInfo.basePos);

#endif

    // Check if player runs against door
#if 0
    for ( int i = 0; i < m_World.m_BrushEntities.size(); i++ ) {
        int be = m_World.m_BrushEntities[ i ];
        BaseGameEntity* pEntity = m_pEntityManager->GetEntityFromID(be);
        if ( pEntity->Type() == ET_DOOR ) {
            Door* pDoor = (Door*)pEntity;
            CollisionInfo ciDoor = PushTouch(ec,
                                             static_cast<float>(dt) * m_pPlayerEntity->GetVelocity(),
                                             pDoor->TriPlanes().data(),
                                             pDoor->TriPlanes().size());
            if ( ciDoor.didCollide ) {
                printf("COLLIDED!\n");
                Dispatcher->DispatchMessage(
                    SEND_MSG_IMMEDIATELY, m_pPlayerEntity->ID(), pDoor->ID(), message_type::Collision, 0);
            }
        }
    }
#endif

    // Run the message system
    m_pEntityManager->UpdateEntities();
    Dispatcher->DispatchDelayedMessages();

    // Fix camera position

    m_pPlayerEntity->UpdateCamera(&m_FollowCamera);

    // Render stuff

    // ImGUI stuff goes into GL default FBO
    m_Renderer->RenderBegin();

    ImGui::ShowDemoWindow();

    // Main 3D: This is where all the 3D rendering happens (in its own FBO)
    {
        m_Renderer->Begin3D();

        // Draw Debug Line for player veloctiy vector
        Line velocityDebugLine = { Vertex(ecEnemy.center), Vertex(ecEnemy.center + 50.0f * enemy->GetVelocity()) };
        velocityDebugLine.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        velocityDebugLine.b.color = velocityDebugLine.a.color;
        m_Renderer->ImDrawLines(velocityDebugLine.vertices, 2, false);

        // Render World geometry
        m_Renderer->ImDrawTriPlanes(m_World.m_TriPlanes.data(), m_World.m_TriPlanes.size(), true, DRAW_MODE_SOLID);

        // Render Brush Entities
        for ( int i = 0; i < m_World.m_BrushEntities.size(); i++ ) {
            int be = m_World.m_BrushEntities[ i ];
            BaseGameEntity* pEntity = m_pEntityManager->GetEntityFromID(be);
            if ( pEntity->Type() == ET_DOOR ) {
                Door* pDoor = (Door*)pEntity;
                m_Renderer->ImDrawTriPlanes(
                    pDoor->TriPlanes().data(), pDoor->TriPlanes().size(), true, DRAW_MODE_SOLID);
            }
        }

        DrawCoordinateSystem(m_Renderer);

        // auto type = enemy->m_Type;

        HKD_Model* models[ 2 ] = { m_pPlayerEntity->GetModel(), enemy->GetModel() };
        m_Renderer->Render(&m_FollowCamera, models, 2);

#if 0
        if ( collisionInfo.didCollide ) {
            m_pPlayerEntity->GetModel()->debugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
        } else {
            m_pPlayerEntity->GetModel()->debugColor = glm::vec4(1.0f); // white
        }
#endif

#if 0 // Toggle draw hitpoint
        m_Renderer->SetActiveCamera(&m_FollowCamera);
        m_Renderer->ImDrawSphere(collisionInfo.hitPoint, 5.0f);
#endif

        // Render Player's ellipsoid collider
        m_Renderer->SetActiveCamera(&m_FollowCamera);
        HKD_Model* playerColliderModel[ 1 ] = { m_pPlayerEntity->GetModel() };
        m_Renderer->RenderColliders(&m_FollowCamera, playerColliderModel, 1);

        m_Renderer->End3D();

    } // End3D scope

#if 0 // Toggle 2D Font/Box renderingtest

    // Usage example of 2D Screenspace Rendering (useful for UI, HUD, Console...)
    // 2D stuff also has its own, dedicated FBO!
    {
        m_Renderer
            ->Begin2D(); // Enable screenspace 2D rendering. Binds the 2d offscreen framebuffer and activates the 2d shaders.

        //m_Renderer->DrawBox( 10, 20, 200, 200, glm::vec4(0.4f, 0.3f, 1.0f, 1.0f) );
        m_Renderer->SetFont(m_ConsoleFont, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_Renderer->DrawText("ABCDEFGHIJKLMNOajdidjST*~`!/]}]|!#@#=;'\"$%%^&*():L", 0.0f, 0.0f);

        // If you want to draw in absolute coordinates then you have to specify it.
        // Depends on the resolution of the render window!
        m_Renderer->SetFont(m_ConsoleFont, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        m_Renderer->DrawText("Some more text in yellow :)", 0.0f, 200.0f, COORD_MODE_ABS);
        m_Renderer->DrawText("And blended with box on top", 100.0f, 300.0f, COORD_MODE_ABS);

        m_Renderer->SetShapeColor(glm::vec4(0.7f, 0.3f, 0.7f, 0.7));
        m_Renderer->DrawBox(200.0f, 200.0f, 800.0f, 200.0f, COORD_MODE_ABS);

        m_Renderer->SetShapeColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        m_Renderer->DrawBox(0.5f, 0.5f, 0.25f, 0.5f);

        m_Renderer->SetFont(m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f));

        // Use Relative coords (the default). Independent from screen resolution.
        // Goes from 0 (top/left) to 1 (bottom/right).
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 0.5f, 0.5f);

        m_Renderer->SetShapeColor(glm::vec4(0.3f, 0.3f, 0.7f, 1.0));
        m_Renderer->DrawBox(600, 600, 200, 100, COORD_MODE_ABS);
        m_Renderer->SetFont(m_ConsoleFont30, glm::vec4(0.3f, 1.0f, 0.6f, 1.0f));
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 600.0f, 600.0f, COORD_MODE_ABS);
        m_Renderer->SetFont(m_ConsoleFont30, glm::vec4(0.0f, 0.0f, 1.0f, 0.5f));
        m_Renderer->DrawText("Waaay smaller text here!!!! (font size 30)", 605.0f, 605.0f, COORD_MODE_ABS);

        m_Renderer->End2D(); // Stop 2D mode. Unbind 2d offscreen framebuffer.
    } // End2D Scope
#endif

    // This call composits 2D and 3D together into the default FBO
    // (along with ImGUI).
    m_Renderer->RenderEnd();

    return true;
}

void Game::Shutdown() {
    m_Renderer->Shutdown();
    delete m_Renderer;

    // NOTE: m_pEntityManager is static memory and cannot deleted with delete
    // (it never was heap allocated with 'new'). The entities have to
    // be released manually.
    // delete m_pEntityManager;
    m_pEntityManager->KillEntities();
}
