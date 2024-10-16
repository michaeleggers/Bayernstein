//
// Created by benek on 10/16/24.
//

#include "Clock.h"
#include "SDL_timer.h"

Clock* Clock::Instance() {
  static Clock instance;

  return &instance;

}

double Clock::GetTime() {
  return SDL_GetTicks();
}

