#include "Audio.h"

#include <stdlib.h>
#include <string>
#include <vector>

#include "../Console/CommandManager.h"
#include "../Console/Console.h"
#include "../utils/utils.h"


void SetVolume_f(std::vector<std::string> args) {
    std::string cmd = args[ 0 ];
    if ( args.size() != 2 ) {
        Console::Print("Unsupported number of arguments!");
    }

    std::string arg = args[ 1 ];
    if ( !IsStringFloat(arg) ) {
        Console::Printf("Can't set volume, invalid value '%s'.", arg);
    } else {
        printf("volume cmd: %s\n", cmd.c_str());
        float val = atof(arg.c_str());
        if ( cmd == "set_volume_music" ) {
            Audio::m_Soloud.setVolume(Audio::m_MusicBusHandle, val);
        } else if ( cmd == "set_volume_ambience" ) {
            Audio::m_Soloud.setVolume(Audio::m_AmbienceBusHandle, val);
        } else if ( cmd == "set_volume_sfx" ) {
            Audio::m_Soloud.setVolume(Audio::m_SfxBusHandle, val);
        } else if ( cmd == "set_volume" ) {
            Audio::m_Soloud.setGlobalVolume(val);
        }
    }
}


SoLoud::Soloud Audio::m_Soloud;

SoLoud::Bus    Audio::m_MusicBus;
SoLoud::handle Audio::m_MusicBusHandle;

SoLoud::Bus    Audio::m_AmbienceBus;
SoLoud::handle Audio::m_AmbienceBusHandle;

SoLoud::Bus    Audio::m_SfxBus;
SoLoud::handle Audio::m_SfxBusHandle;

bool Audio::Init() {
    if ( m_Soloud.init() != SoLoud::SO_NO_ERROR ) {
        return false;
    }

    m_MusicBusHandle    = m_Soloud.play(m_MusicBus);
    m_AmbienceBusHandle = m_Soloud.play(m_AmbienceBus);
    m_SfxBusHandle      = m_Soloud.play(m_SfxBus);

    CommandManager::Add("set_volume", SetVolume_f);
    CommandManager::Add("set_volume_music", SetVolume_f);
    CommandManager::Add("set_volume_ambience", SetVolume_f);
    CommandManager::Add("set_volume_sfx", SetVolume_f);
    // TODO: ^^ should these be `ConsoleVariable`s with onChange callbacks (similar to some vars in quake)?

    return true;
}

void Audio::Deinit() {
    m_Soloud.deinit();
}
