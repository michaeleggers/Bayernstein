#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

// Holds SDL mouse-motion state for current and previous frame.
struct MouseMotion {
    SDL_MouseMotionEvent current;
    SDL_MouseMotionEvent prev;
};

void HandleInput(void);
bool KeyWentDown(SDL_Keycode keyCode);
bool KeyWentUp(SDL_Keycode keyCode);
bool KeyPressed(SDL_Keycode keyCode);
bool MouseWentDown(int button);
bool MouseWentUp(int button);
bool MousePressed(int button);
bool RightMouseWentDown(void);
const MouseMotion GetMouseMotion(void);
bool ShouldClose(void);

#endif
