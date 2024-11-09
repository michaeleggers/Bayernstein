
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
#include "utils.h"
#include "Console/VariableManager.h"

static bool         g_GameWantsToQuit;
std::string         g_GameDir;
static IRender*     g_Renderer;

static bool QuitGameFunc(void) {
    g_GameWantsToQuit = true;
    return true;
}

static double msPerFrame;
double GetDeltaTime() {
    return msPerFrame;
}

IRender* GetRenderer() {
    return g_Renderer;
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


    g_Renderer = new GLRender();
    if (!g_Renderer->Init()) {
        SDL_Log("Could not initialize renderer.\n");
        return -1;
    }    

    VariableManager::Init();
    Console* console = Console::Create(100, 32);
    CFont* consoleFont = new CFont("fonts/HackNerdFont-Bold.ttf", 26);
    g_Renderer->RegisterFont(consoleFont);

    // Init the game

    Game game(exePath, &interface);
    game.Init();    
    
    // Main loop
    
    Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64 startCounter = SDL_GetPerformanceCounter();
    Uint64 endCounter = SDL_GetPerformanceCounter();

    float updateIntervalMs = 0.0f;
    while (!ShouldClose() && !g_GameWantsToQuit) {
        double ticksPerFrame = (double)endCounter - (double)startCounter;
        msPerFrame = (ticksPerFrame / (double)ticksPerSecond) * 1000.0;

        startCounter = SDL_GetPerformanceCounter();

        HandleInput();

        if (TextInput() == "^") { // caret is not a standalone key in german keyboard layout (`KeyWentDown(SDLK_CARET)` doesn't work)
            console->m_isActive = !console->m_isActive;
            console->m_blinkTimer = 0;
        } else if (console->m_isActive) {
            console->m_blinkTimer += msPerFrame;
            if (TextInput().length()) {
                console->UpdateInput(TextInput());
            } else if (KeyWentDown(SDLK_UP)) { // TODO: add support for 'repeat' mode
                console->SetInputFromHistory(1);
            } else if (KeyWentDown(SDLK_DOWN)) {
                console->SetInputFromHistory(-1);
            } else if (KeyWentDown(SDLK_LEFT)) {
                console->MoveCursor(-1);
            } else if (KeyWentDown(SDLK_RIGHT)) {
                console->MoveCursor(1);
            } else if (KeyWentDown(SDLK_BACKSPACE)) {
                console->DeleteInput(-1);
            } else if (KeyWentDown(SDLK_DELETE)) {
                console->DeleteInput(1);
            } else if (KeyWentDown(SDLK_RETURN)) {
                console->SubmitInput();
            }
            g_Renderer->RenderBegin();
            g_Renderer->RenderConsole(console, consoleFont);
            g_Renderer->RenderEnd();
        } else { // FIXME: the game's 2d content disappears while console is open
            game.RunFrame(msPerFrame);
        }

        //printf("msPerFrame: %f\n", msPerFrame);
        //printf("FPS: %f\n", 1000.0f/msPerFrame);

        // Update window title every second.
        updateIntervalMs += msPerFrame;
        if (updateIntervalMs >= 1000.0f) {
            char windowTitle[256];
            sprintf(windowTitle,
                    "Device: %s, frametime (ms): %f, FPS: %f",
                    glGetString(GL_RENDERER), msPerFrame, 1000.0f / msPerFrame);
            g_Renderer->SetWindowTitle(windowTitle);
            updateIntervalMs = 0.0f;
        }

        endCounter = SDL_GetPerformanceCounter();
    }

    game.Shutdown();

    g_Renderer->Shutdown();

    // Clean up
    SDL_Quit();

    return 0;
}
