#include "Audio.h"

#include <stdlib.h>
#include <string>
#include <vector>

#include "../Console/CommandManager.h"
#include "../Console/Console.h"
#include "../utils/utils.h"

void GetVolume_f(std::vector<std::string> _args) {
    float global   = Audio::m_Soloud.getGlobalVolume();
    float sfx      = Audio::m_Soloud.getVolume(Audio::m_SfxBusHandle);
    float ambience = Audio::m_Soloud.getVolume(Audio::m_AmbienceBusHandle);
    float music    = Audio::m_Soloud.getVolume(Audio::m_MusicBusHandle);
    Console::Printf("global: %.2f  ---  sfx: %.2f  ---  ambiance: %.2f  ---  music: %.2f", global, sfx, ambience, music);
}

void SetVolume_f(std::vector<std::string> args) {
    std::string bus;
    std::string arg;
    if ( args.size() == 2 ) {
        bus = "global";
        arg = args[ 1 ];
    } else if ( args.size() == 3 ) {
        bus = args[ 1 ];
        arg = args[ 2 ];
    } else {
        Console::Print("Unsupported number of arguments!");
        return;
    }

    if ( !IsStringFloat(arg) ) {
        Console::Printf("Can't set volume, invalid value '%s'.", arg);
    } else {
        float val = atof(arg.c_str());
        if ( bus == "music" ) {
            Audio::m_Soloud.setVolume(Audio::m_MusicBusHandle, val);
        } else if ( bus == "ambience" ) {
            Audio::m_Soloud.setVolume(Audio::m_AmbienceBusHandle, val);
        } else if ( bus == "sfx" ) {
            Audio::m_Soloud.setVolume(Audio::m_SfxBusHandle, val);
        } else if ( bus == "global" ) {
            Audio::m_Soloud.setGlobalVolume(val);
        } else {
            Console::Printf("Unsupported audio bus '%s', use: global, sfx, ambience, music", bus);
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

    CommandManager::Add("get_volume", GetVolume_f);
    CommandManager::Add("set_volume", SetVolume_f);

    return true;
}

void Audio::Deinit() {
    m_Soloud.deinit();
}
