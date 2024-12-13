// TODO: Maybe put them into a game-independent file later.
#include "VariableManager.h"
ConsoleVariable dbg_show_wander         = { "dbg_show_wander", 0 };
ConsoleVariable dbg_show_enemy_velocity = { "dbg_show_enemy_velocity", 0 };
ConsoleVariable dbg_show_enemy_vision   = { "dbg_show_enemy_vision", 0 };
ConsoleVariable dbg_show_paths          = { "dbg_show_paths", 0 };

void InitDebugVariables() {
    VariableManager::Register(&dbg_show_wander);
    VariableManager::Register(&dbg_show_enemy_velocity);
    VariableManager::Register(&dbg_show_enemy_vision);
    VariableManager::Register(&dbg_show_paths);
}