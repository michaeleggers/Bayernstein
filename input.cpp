#include "input.h"

#include <stdio.h>

#include <SDL.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

static Uint32      g_Events;
static bool        g_Scancodes[ SDL_NUM_SCANCODES ];
static bool        g_PrevScancodes[ SDL_NUM_SCANCODES ];
static bool        g_PerFrameScancodes[ SDL_NUM_SCANCODES ];
static Uint8       g_MouseButtons;
static Uint8       g_PrevMouseButtons;
static MouseMotion g_MouseMotionState;
static MouseWheel  g_MouseWheelState;

static std::string g_EventText;

void HandleInput(void) {
    memcpy(g_PrevScancodes, g_Scancodes, SDL_NUM_SCANCODES * sizeof(bool));
    g_PrevMouseButtons = g_MouseButtons;
    memset(g_PerFrameScancodes, 0, SDL_NUM_SCANCODES * sizeof(bool));

    g_Events                  = 0;
    g_EventText               = "";
    g_MouseWheelState.updated = false;

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {

        ImGui_ImplSDL2_ProcessEvent(&event);

        if ( event.type == SDL_QUIT ) {
            g_Events |= SDL_QUIT;
        }
        if ( event.type == SDL_WINDOWEVENT ) {
            g_Events |= SDL_WINDOWEVENT;
        }

        if ( event.type == SDL_KEYDOWN ) {
            g_Scancodes[ event.key.keysym.scancode ]         = true;
            g_PerFrameScancodes[ event.key.keysym.scancode ] = true;
        }
        if ( event.type == SDL_KEYUP ) {
            g_Scancodes[ event.key.keysym.scancode ]         = false;
            g_PerFrameScancodes[ event.key.keysym.scancode ] = false;
        }

        if ( event.type == SDL_MOUSEBUTTONDOWN ) {
            g_MouseButtons |= SDL_BUTTON(event.button.button);
        }
        if ( event.type == SDL_MOUSEBUTTONUP ) {
            g_MouseButtons ^= SDL_BUTTON(event.button.button);
        }
        if ( event.type == SDL_MOUSEMOTION ) {
            g_MouseMotionState.prev    = g_MouseMotionState.current;
            g_MouseMotionState.current = event.motion;
        }
        if ( event.type == SDL_MOUSEWHEEL ) {
            g_MouseWheelState.prev    = g_MouseWheelState.current;
            g_MouseWheelState.current = event.wheel;
            g_MouseWheelState.updated = true;
        }

        if ( event.type == SDL_TEXTINPUT ) {
            auto str = std::string(event.text.text);
            if ( str.length() == 1 ) g_EventText = str; // ignore unicode characters (length > 1)
        }
    }
}

bool KeyWentDown(SDL_Keycode keyCode) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (!g_PrevScancodes[ sc ] && g_Scancodes[ sc ]);
}

bool KeyWentDownUnbuffered(SDL_Keycode keyCode) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return g_PerFrameScancodes[ sc ];
}

bool KeyWentUp(SDL_Keycode keyCode) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (g_PrevScancodes[ sc ] && !g_Scancodes[ sc ]);
}

bool KeyPressed(SDL_Keycode keyCode) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(keyCode);
    return (g_PrevScancodes[ sc ] && g_Scancodes[ sc ]);
}

bool MouseWentDown(int button) {
    if ( button > SDL_BUTTON_RIGHT || button < SDL_BUTTON_LEFT ) {
        return false;
    }
    Uint8 uButton = (Uint8)button;

    return (!(g_PrevMouseButtons & SDL_BUTTON(uButton)) && (g_MouseButtons & SDL_BUTTON(uButton)));
}

bool MouseWentUp(int button) {
    if ( button > SDL_BUTTON_RIGHT || button < SDL_BUTTON_LEFT ) {
        return false;
    }
    Uint8 uButton = (Uint8)button;

    return ((g_PrevMouseButtons & SDL_BUTTON(uButton)) && !(g_MouseButtons & SDL_BUTTON(uButton)));
}

bool MousePressed(int button) {
    if ( button > SDL_BUTTON_RIGHT || button < SDL_BUTTON_LEFT ) {
        return false;
    }
    Uint8 uButton = (Uint8)button;

    return g_MouseButtons & SDL_BUTTON(uButton);
}

bool RightMouseWentDown(void) {
    return false;
}

const MouseMotion GetMouseMotion(void) {
    return g_MouseMotionState;
}

const MouseWheel GetMouseWheel(void) {
    return g_MouseWheelState;
}

bool ShouldClose(void) {
    return g_Events & SDL_QUIT;
}

const std::string& TextInput() {
    return g_EventText;
}

void ClearTextInput() {
    g_EventText = "";
}
