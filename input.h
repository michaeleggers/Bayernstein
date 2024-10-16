#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

void HandleInput();
bool KeyWentDown(SDL_Keycode keyCode);
bool KeyWentUp(SDL_Keycode keyCode);
bool KeyPressed(SDL_Keycode keyCode);
bool MouseWentDown(Uint8 button);
bool MouseWentUp(Uint8 button);
bool MousePressed(Uint8 button);
bool RightMouseWentDown();
bool ShouldClose();

#endif
