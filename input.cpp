#include "input.h"

#include <stdio.h>

#include <SDL.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

static Uint32 g_Events;
static bool   g_Scancodes[SDL_NUM_SCANCODES];
static bool   g_PrevScancodes[SDL_NUM_SCANCODES];
static Uint8  g_MouseButtons;
static Uint8  g_PrevMouseButtons;

void HandleInput(void)
{
    memcpy(g_PrevScancodes, g_Scancodes, SDL_NUM_SCANCODES * sizeof(bool));
    g_PrevMouseButtons = g_MouseButtons;
    //memset(g_Scancodes, 0, SDL_NUM_SCANCODES * sizeof(bool));

    g_Events = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            g_Events |= SDL_QUIT;
        }
        if (event.type == SDL_WINDOWEVENT) {
            g_Events |= SDL_WINDOWEVENT;
        }

        if (event.type == SDL_KEYDOWN) {
            g_Scancodes[event.key.keysym.scancode] = true;            
        }
        if (event.type == SDL_KEYUP) {
            g_Scancodes[event.key.keysym.scancode] = false;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            g_MouseButtons |= SDL_BUTTON(event.button.button);
        }
        if (event.type == SDL_MOUSEBUTTONUP) {
            g_MouseButtons ^= SDL_BUTTON(event.button.button);
        }
    }
}

bool KeyWentDown(SDL_Keycode keyCode)
{
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (!g_PrevScancodes[sc] && g_Scancodes[sc]);
}

bool KeyWentUp(SDL_Keycode keyCode)
{
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (g_PrevScancodes[sc] && !g_Scancodes[sc]);
}

bool KeyPressed(SDL_Keycode keyCode)
{
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (g_PrevScancodes[sc] && g_Scancodes[sc]);
}

bool MouseWentDown(Uint8 button)
{
    return (!(g_PrevMouseButtons & SDL_BUTTON(button)) && (g_MouseButtons & SDL_BUTTON(button)));
}

bool MouseWentUp(Uint8 button)
{
    return ((g_PrevMouseButtons & SDL_BUTTON(button)) && !(g_MouseButtons & SDL_BUTTON(button)));
}

bool MousePressed(Uint8 button)
{
    return g_MouseButtons & SDL_BUTTON(button);
}

bool RightMouseWentDown(void)
{
    return false;
}

bool ShouldClose(void)
{    
    return g_Events & SDL_QUIT;
}
