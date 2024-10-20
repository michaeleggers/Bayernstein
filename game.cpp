#include "game.h"

#include <SDL.h>
#include <string>

#include "CWorld.h"
#include "Shape.h"
#include "ShapeSphere.h"
#include "camera.h"
#include "input.h"
#include "physics.h"
#include "r_itexture.h"
#include "utils.h"

#include "imgui.h"

#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"

#include "polysoup.h"

static int hkd_Clamp(int val, int clamp) {
	if (val > clamp || val < clamp) return clamp;
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
		MapPolygon mapPoly = tris[i];
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
		worldTris.push_back(triPlane);
	}
	m_World.InitWorld(worldTris.data(), worldTris.size(), glm::vec3(0.0f, 0.0f, -0.5f));

	glm::vec3 startPositionPlayer = getPlayerStartPosition(&map);

	m_pEntityManager->RegisterEntity(new Player(startPositionPlayer), &m_World);
	Player* player = m_pEntityManager->GetPlayerEntity();
	HKD_Model* playerModel = player->GetModel();
	int hPlayerModel = m_Renderer->RegisterModel(playerModel);

	m_pEntityManager->RegisterEntity(new Enemy(1), &m_World);

	m_FollowCamera = Camera(m_Player.position);
	m_FollowCamera.m_Pos.y -= 200.0f;
	m_FollowCamera.m_Pos.z += 100.0f;
	m_FollowCamera.RotateAroundSide(-20.0f);
	m_FollowCamera.RotateAroundUp(180.0f);
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
	m_pEntityManager->UpdateEntities(dt);
	Player* player = m_pEntityManager->GetPlayerEntity();
	player->UpdateCamera(&m_FollowCamera);

	// Render stuff
	m_Renderer->RenderBegin();

	ImGui::ShowDemoWindow();

	// Render World geometry
	m_Renderer->ImDrawTriPlanes(m_World.m_TriPlanes.data(), m_World.m_TriPlanes.size(), true, DRAW_MODE_SOLID);

	DrawCoordinateSystem(m_Renderer);
	player->Render(m_Renderer, &m_FollowCamera);

	m_Renderer->RenderEnd();

	Dispatcher->DispatchDelayedMessages();

	return true;
}

void Game::Shutdown() {
	m_Renderer->Shutdown();
	delete m_Renderer;
	delete m_pEntityManager;
}
