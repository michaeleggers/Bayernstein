#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

void HandleInput(void);
bool KeyWentDown(SDL_Keycode keyCode);
bool KeyWentUp(SDL_Keycode keyCode);
bool KeyPressed(SDL_Keycode keyCode);
bool MouseWentDown(int button);
bool MouseWentUp(int button);
bool MousePressed(int button);
bool RightMouseWentDown(void);
bool ShouldClose(void);

#endif
