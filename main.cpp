
// HKD Game - 20.1.2024 - Michael Eggers

#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string>

#include <SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "platform.h"
#include "iqm_loader.h"
#include "irender.h"
#include "r_gl.h"
#include "r_model.h"
#include "input.h"
#include "hkd_interface.h"
#include "physics.h"
#include "game.h"
#include "TestClass.h"

static bool         g_GameWantsToQuit;
std::string         g_GameDir;


static bool QuitGameFunc(void) {
    g_GameWantsToQuit = true;
    return true;
}

int main(int argc, char** argv)
{

#if WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif

    // Init globals

    g_GameWantsToQuit = false;

    // Init subsystems

    hkdInterface interface = {};
    interface.QuitGame = QuitGameFunc;

    std::string exePath = hkd_GetExePath();
    interface.gameDir = exePath;
    g_GameDir = exePath;
    if (argc > 1) {
        interface.gameDir += std::string(argv[1]);
        g_GameDir += std::string(argv[1]);
    }


    IRender* renderer = new GLRender();
    if (!renderer->Init()) {
        SDL_Log("Could not initialize renderer.\n");
        return -1;
    }    

    // Init the game

    Game game(exePath, &interface, renderer);
    game.Init();    
    
    // Main loop
    
    Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64 startCounter = SDL_GetPerformanceCounter();
    Uint64 endCounter = SDL_GetPerformanceCounter();

    float updateIntervalMs = 0.0f;
    while (!ShouldClose() && !g_GameWantsToQuit) {
        double ticksPerFrame = (double)endCounter - (double)startCounter;
        double msPerFrame = (ticksPerFrame / (double)ticksPerSecond) * 1000.0;

        startCounter = SDL_GetPerformanceCounter();

        HandleInput();

        game.RunFrame(msPerFrame);

        //printf("msPerFrame: %f\n", msPerFrame);
        //printf("FPS: %f\n", 1000.0f/msPerFrame);

        // Update window title every second.
        updateIntervalMs += msPerFrame;
        if (updateIntervalMs >= 1000.0f) {
            char windowTitle[256];
            sprintf(windowTitle,
                    "Device: %s, frametime (ms): %f, FPS: %f",
                    glGetString(GL_RENDERER), msPerFrame, 1000.0f / msPerFrame);
            renderer->SetWindowTitle(windowTitle);
            updateIntervalMs = 0.0f;
        }

        endCounter = SDL_GetPerformanceCounter();
    }

    game.Shutdown();

    // Clean up
    SDL_Quit();

    return 0;
}
