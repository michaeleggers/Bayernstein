
// HKD Game - 20.1.2024 - Michael Eggers

#if WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Audio/Audio.h"
#include "Console/VariableManager.h"
#include "TestClass.h"
#include "game.h"
#include "hkd_interface.h"
#include "input.h"
#include "iqm_loader.h"
#include "irender.h"
#include "physics.h"
#include "platform.h"
#include "r_gl.h"
#include "r_model.h"
#include "utils/utils.h"

static bool     g_GameWantsToQuit;
std::string     g_GameDir;
static IRender* g_Renderer;

static bool QuitGameFunc(void) {
    g_GameWantsToQuit = true;
    return true;
}

static double msPerFrame;
double        GetDeltaTime() {
    return msPerFrame;
}

IRender* GetRenderer() {
    return g_Renderer;
}

static DebugSettings g_debugSettings;



DebugSettings* GetDebugSettings()
{
    return &g_debugSettings;
}

int main(int argc, char** argv) {

#if WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif

    // Init globals

    g_GameWantsToQuit = false;

    // Init subsystems

    hkdInterface interface = {};
    interface.QuitGame     = QuitGameFunc;

    std::string exePath = hkd_GetExePath();
    interface.gameDir   = exePath;
    g_GameDir           = exePath;
    if ( argc > 1 ) {
        interface.gameDir += std::string(argv[ 1 ]);
        g_GameDir += std::string(argv[ 1 ]);
    }

    g_Renderer = new GLRender();
    if ( !g_Renderer->Init() ) {
        SDL_Log("Could not initialize renderer.\n");
        return -1;
    }

    VariableManager::Init();
    Console* console = Console::Create(100, 32);

    if ( !Audio::Init() ) {
        SDL_Log("Could not initialize audio engine.\n");
        return -1;
    }

    // Init the game

    Game game(exePath, &interface);
    game.Init();

    // Main loop

    Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64 startCounter   = SDL_GetPerformanceCounter();
    Uint64 endCounter     = SDL_GetPerformanceCounter();

    float updateIntervalMs = 0.0f;
    while ( !ShouldClose() && !g_GameWantsToQuit ) {
        double ticksPerFrame = (double)endCounter - (double)startCounter;
        msPerFrame           = (ticksPerFrame / (double)ticksPerSecond) * 1000.0;

        startCounter = SDL_GetPerformanceCounter();

        HandleInput();

        g_Renderer->RenderBegin();

        // caret is not a standalone key in german keyboard layout (`KeyWentDown(SDLK_CARET)` doesn't work)
        if ( TextInput() == "^" ) {
            console->m_isActive   = !(console->m_isActive);
            console->m_blinkTimer = 0;
            ClearTextInput(); // Remove caret from the buffer
        }
        if ( console->m_isActive ) {
            // FIXME: the game's 2d content disappears while console is open
            // NOTE: (Michael): We let this unfixed for now and defer this
            // to another time. To implement this cleanly a few things
            // in the renderer have to be changed, eg. refactoring the
            // render-internal render-command API. Since this API has
            // to be changed anyways for regular 3D drawing I want to
            // see what other requirements this change needs before
            // investing too much time now and having to change everything
            // later...
            console->RunFrame();
        } else {
            game.RunFrame(msPerFrame);
        }

        // This call composits 2D and 3D together into the default FBO
        // (along with ImGUI).
        g_Renderer->RenderEnd();

        //printf("msPerFrame: %f\n", msPerFrame);
        //printf("FPS: %f\n", 1000.0f/msPerFrame);

        // Update window title every second.
        updateIntervalMs += msPerFrame;
        if ( updateIntervalMs >= 1000.0f ) {
            char windowTitle[ 256 ];
            sprintf(windowTitle,
                    "Device: %s, frametime (ms): %f, FPS: %f",
                    glGetString(GL_RENDERER),
                    msPerFrame,
                    1000.0f / msPerFrame);
            g_Renderer->SetWindowTitle(windowTitle);
            updateIntervalMs = 0.0f;
        }

        endCounter = SDL_GetPerformanceCounter();
    }

    Audio::Deinit();

    game.Shutdown();

    g_Renderer->Shutdown();

    // Clean up
    SDL_Quit();

    return 0;
}
