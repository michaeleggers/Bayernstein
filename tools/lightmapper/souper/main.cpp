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
#include <image.h>
#include <platform.h>
#include <polysoup.h>

#include <stdio.h>
#include <string>

using json = nlohmann::json;

// TODO: Virtual file system
// NOTE: We need to specify g_GameDir because in image.cpp (which this porgram
// relies on) it is being used and expected to be set.
std::string g_GameDir;

void to_json(json& j, const QuakeMapVertex& vertex) {
    j = json{ { "pos", { vertex.pos.x, vertex.pos.y, vertex.pos.z } }, { "uv", { vertex.uv.x, vertex.uv.y } } };
}

void to_json(json& j, const MapPolygon& polygon) {
    j = json{ { "vertices", polygon.vertices },
              { "normal", { polygon.normal.x, polygon.normal.y, polygon.normal.z } },
              { "textureName", polygon.textureName },
              { "surfaceFlags", polygon.surfaceFlags },
              { "contentFlags", polygon.contentFlags } };
}

void saveMapToJson(const std::string&             filename,
                   const std::vector<MapPolygon>& polygons,
                   const std::vector<PointLight>& lights) {
    // Create a JSON object to hold both polygons and lights
    json j;

    // Convert polygons to JSON
    j[ "polygons" ] = polygons;

    // Convert lights to JSON
    j[ "lights" ] = json::array();
    for ( const auto& light : lights ) {
        j[ "lights" ].push_back({ { "origin", light.origin },
                                  { "intensity", light.intensity },
                                  { "color", light.color },
                                  { "range", light.range } });
    }

    // Write JSON to file
    std::ofstream file(filename);
    if ( file.is_open() ) {
        file << j.dump(4); // Pretty print with an indentation of 4 spaces
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }
}
static std::string getPropertyValue(const std::vector<Property>& properties,
                                    const std::string&           key,
                                    const std::string&           defaultValue = "") {
    for ( const auto& p : properties ) {
        if ( p.key == key ) {
            return p.value;
        }
    }

    if ( !defaultValue.empty() ) {
        return defaultValue;
    }

    throw std::runtime_error("Property key not found: " + key);
}

// this function already exists in polysoup
static bool hasClassname(const Entity& e, std::string classname) {
    for ( int i = 0; i < e.properties.size(); i++ ) {
        const Property& p = e.properties[ i ];
        if ( p.value == classname ) {
            return true;
        }
    }
    return false;
}

std::vector<PointLight> extractPointLights(const Map& map) {
    std::vector<PointLight> pointLights;

    for ( const auto& entity : map.entities ) {
        // Check if entity has classname "light"
        if ( !hasClassname(entity, "light") ) {
            continue;
        }

        try {
            PointLight light;

            // Extract fields from properties with optional defaults
            light.origin    = getPropertyValue(entity.properties, "origin");
            light.intensity = std::stof(getPropertyValue(entity.properties, "light", "1")); // Default intensity = 1.0
            light.color     = getPropertyValue(entity.properties, "color", "1.0 1.0 1.0");  // Default color = white
            light.range     = std::stof(getPropertyValue(entity.properties, "range", "256.0")); // Default range = 256.0

            // Add to the list
            pointLights.push_back(light);
        } catch ( const std::exception& e ) {
            // Handle missing or malformed fields gracefully
            std::cerr << "Error parsing light entity: " << e.what() << std::endl;
        }
    }

    return pointLights;
}

int main(int argc, char** argv) {

    if ( argc < 4 ) {
        printf("Usage: soup <game-dir> <MAP-file> <out-file>\nBye!\n");
        return -1;
    }

    g_GameDir           = std::string(argv[ 1 ]);
    std::string mapFile = argv[ 2 ];
    std::string outFile = argv[ 3 ];
    std::string exePath = hkd_GetExePath();

    std::string fullPath = mapFile;
    std::string mapData  = loadTextFile(fullPath);

    size_t                  inputLength = mapData.length();
    Map                     map         = getMap(&mapData[ 0 ], inputLength, VALVE_220);
    std::vector<MapPolygon> polysoup    = createPolysoup(map);
    std::vector<MapPolygon> tris        = triangulate(polysoup);
    std::vector<PointLight> lights      = extractPointLights(map);

    // TODO: Fabi makes this work :)
    std::vector<BrushEntity> brushEntities{};
    for ( auto& entity : map.entities ) {
        std::vector<Brush>&     brushes   = entity.brushes;
        Brush&                  brush     = brushes[ 0 ];
        std::vector<MapPolygon> doorPolys = createPolysoup(brush);
        std::vector<MapPolygon> doorTris  = triangulate(doorPolys);
        brushEntities.push_back({ entity.properties, doorTris });
    }

// Print UV coordinates for debugging
#if 0
    for (const auto& poly : tris) {
        std::cout << "Polygon with texture: " << poly.textureName << "\n";
        for (const auto& vertex : poly.vertices) {
            std::cout << "UV: (" << (float)vertex.uv.x << ", " << (float)vertex.uv.y << ")\n";
        }
        std::cout << "------\n";
    }
#endif

    //writePolySoupBinary(outFile, tris);
    try {
        saveMapToJson(outFile, tris, lights);
    } catch ( const std::exception& e ) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
