#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>
#include <string>

// Holds SDL mouse-motion state for current and previous frame.
struct MouseMotion {
    SDL_MouseMotionEvent current;
    SDL_MouseMotionEvent prev;
};

// Holds SDL mouse-wheel state for current and previous frame.
struct MouseWheel {
    SDL_MouseWheelEvent current;
    SDL_MouseWheelEvent prev;
    bool updated;
};

void HandleInput(void);
bool KeyWentDown(SDL_Keycode keyCode);

/* 
* Reflects what the message loop of the
* operating system will do. If you press down the
* key and keep it pressed, you will notice a
* short delay between the initial press and
* the 'keep pressed' state.
*/
bool KeyWentDownUnbuffered(SDL_Keycode keyCode);

bool KeyWentUp(SDL_Keycode keyCode);
bool KeyPressed(SDL_Keycode keyCode);
bool MouseWentDown(int button);
bool MouseWentUp(int button);
bool MousePressed(int button);
bool RightMouseWentDown(void);
const MouseMotion GetMouseMotion(void);
const MouseWheel GetMouseWheel(void);
bool ShouldClose(void);

const std::string&  TextInput();
void                ClearTextInput();

#endif

