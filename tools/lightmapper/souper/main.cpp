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

#include <SDL.h>
#include <polysoup.h>
#include <image.h>
#include <platform.h>

#include <stdio.h>
#include <string>

// TODO: Virtual file system
// NOTE: We need to specify g_GameDir because in image.cpp (which this porgram
// relies on) it is being used and expected to be set.
std::string g_GameDir;

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

    std::string fullPath = g_GameDir + mapFile;
    std::string mapData = loadTextFile(fullPath);

    size_t inputLength = mapData.length();
    Map map = getMap(&mapData[0], inputLength, VALVE_220); 
    std::vector<MapPolygon> polysoup = createPolysoup(map);
    std::vector<MapPolygon> tris = triangulate(polysoup);
    
    writePolySoupBinary(outFile, tris);
   
    return 0;
}

