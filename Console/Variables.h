#ifndef VARIABLES_H
#define VARIABLES_H

#include "VariableManager.h"

extern ConsoleVariable dbg_show_wander;
extern ConsoleVariable dbg_show_enemy_velocity;
extern ConsoleVariable dbg_show_enemy_vision;
extern ConsoleVariable dbg_show_paths;

void InitDebugVariables();

#endif // VARIABLES_H