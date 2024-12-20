#ifndef _HKD_INTERFACE_H_
#define _HKD_INTERFACE_H_

#include "irender.h"

#include <string>

typedef bool (*QUIT_GAME_PFN)(void);

struct hkdInterface {
    QUIT_GAME_PFN QuitGame;
    std::string   gameDir;
};

IRender* GetRenderer();

#endif
