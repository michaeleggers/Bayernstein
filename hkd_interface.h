#ifndef _HKD_INTERFACE_H_
#define _HKD_INTERFACE_H_

#include <string>

typedef bool (*QUIT_GAME_PFN)();

struct hkdInterface {
    QUIT_GAME_PFN QuitGame;
    std::string   gameDir;
};


#endif
