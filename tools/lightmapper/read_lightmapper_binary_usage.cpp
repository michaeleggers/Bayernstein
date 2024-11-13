#define MAP_PARSER_IMPLEMENTATION
#include "map_parser.h"
#include "polysoup.h"
#include <cstring>
#include <windows.h>
#include <string>
#include <iostream>

struct CompiledVertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv_texture;
	glm::vec2 uv_lightmap;
};

struct CompiledTriangle {
	CompiledVertex vertices[3];
	char textureName[256]; // Fixed-size char array
	uint64_t surfaceFlags;
	uint64_t contentFlags;
};


std::vector<CompiledTriangle> loadTriangles(const std::string& filename) {
	std::vector<CompiledTriangle> triangles;

	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return triangles;
	}

	while (file.peek() != EOF) {
		CompiledTriangle triangle;

		// Read vertices
		file.read(reinterpret_cast<char*>(triangle.vertices), sizeof(triangle.vertices));

		// Read fixed-size texture name
		file.read(triangle.textureName, 256);

		// Read surfaceFlags and contentFlags
		file.read(reinterpret_cast<char*>(&triangle.surfaceFlags), sizeof(uint64_t));
		file.read(reinterpret_cast<char*>(&triangle.contentFlags), sizeof(uint64_t));

		// Add to vector
		triangles.push_back(triangle);
	}

	file.close();
	return triangles;
}


int main()
{

	// Load vertices from the binary file
	std::string filename = "D:/data/Informatik/GamesEngineering/Bayernstein/assets/compiled/test_map_open/test_map_open.ply";
	auto triangles = loadTriangles(filename);

	for (const auto& triangle : triangles) {
		std::cout << "Triangle:\n";
		for (const auto& vertex : triangle.vertices) {
			std::cout << "  Position: (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")\n";
			std::cout << "  Normal: (" << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << ")\n";
			std::cout << "  UV Texture: (" << vertex.uv_texture.x << ", " << vertex.uv_texture.y << ")\n";
			std::cout << "  UV Lightmap: (" << vertex.uv_lightmap.x << ", " << vertex.uv_lightmap.y << ")\n";
		}
		std::cout << "Texture Name: " << triangle.textureName << "\n";
		std::cout << "Surface Flags: " << triangle.surfaceFlags << "\n";
		std::cout << "Content Flags: " << triangle.contentFlags << "\n";
		std::cout << "----------------------\n";
	}

	return 0;
}
