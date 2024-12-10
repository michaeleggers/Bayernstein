/* 
 * Michael Eggers, 27. Oct. 2024, meggers@hm.edu
 *
* Exports a polysoup binary file for a given .MAP file.
*
* NOTE: This is very much in development. This might break
* if you give it a pathname for eg. the game-dir (asset-path for now)
* that does not end with a '/' ('\') on Windows it might not
* find the MAP file!
*
* Have fun. Michael.
*/


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define MAP_PARSER_IMPLEMENTATION
#include <map_parser.h>

#include <nlohmann/json.hpp>

#include <SDL.h>
#include <polysoup.h>
#include <image.h>
#include <platform.h>

#include <stdio.h>
#include <string>

using json = nlohmann::json;

// TODO: Virtual file system
// NOTE: We need to specify g_GameDir because in image.cpp (which this porgram
// relies on) it is being used and expected to be set.
std::string g_GameDir;


void to_json(json& j, const QuakeMapVertex& vertex) {
    j = json{
        {"pos", {vertex.pos.x, vertex.pos.y, vertex.pos.z}},
        {"uv", {vertex.uv.x, vertex.uv.y}}
    };
}

void to_json(json& j, const MapPolygon& polygon) {
    j = json{
        {"vertices", polygon.vertices},
        {"normal", {polygon.normal.x, polygon.normal.y, polygon.normal.z}},
        {"textureName", polygon.textureName},
        {"surfaceFlags", polygon.surfaceFlags},
        {"contentFlags", polygon.contentFlags}
    };
}

void saveMapPolygonsToJson(const std::string& filename, const std::vector<MapPolygon>& polygons) {
    // Convert the vector of polygons to JSON
    json j = polygons;

    // Write JSON to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with an indentation of 4 spaces
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }
}

int main(int argc, char** argv) {

    if (argc < 4) {
        printf("Usage: soup <game-dir> <MAP-file> <out-file>\nBye!\n");
        return -1;
    }

    g_GameDir = argv[1];
    std::string mapFile = argv[2];
    std::string outFile = argv[3];
    std::string exePath = hkd_GetExePath();
    g_GameDir = exePath + g_GameDir;

    std::string fullPath = mapFile;
    std::string mapData = loadTextFile(fullPath);

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[0], inputLength, VALVE_220); 
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    std::vector<MapPolygon> tris = triangulate(polysoup);

    // Print UV coordinates for debugging
    for (const auto& poly : tris) {
        std::cout << "Polygon with texture: " << poly.textureName << "\n";
        for (const auto& vertex : poly.vertices) {
            std::cout << "UV: (" << (float)vertex.uv.x << ", " << (float)vertex.uv.y << ")\n";
        }
        std::cout << "------\n";
    }
    
    //writePolySoupBinary(outFile, tris);
    try {
        saveMapPolygonsToJson(outFile, tris);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

