
//
// Created by benek on 10/16/24.
//

#include "Clock.h"
#include "SDL_timer.h"

CrudeTimer* CrudeTimer::Instance()
{
	static CrudeTimer instance;

	return &instance;
}

double CrudeTimer::GetTime() {
	return SDL_GetTicks();
}

