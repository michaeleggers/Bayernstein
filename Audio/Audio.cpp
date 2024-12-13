#include "Audio.h"

#include <stdlib.h>
#include <string>
#include <vector>

#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "../Console/CommandManager.h"
#include "../Console/Console.h"
#include "../globals.h"
#include "../utils/utils.h"

extern std::string g_GameDir;

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

std::unordered_map<std::string, SoLoud::AudioSource*> Audio::m_Sources;

bool Audio::Init() {
    if ( m_Soloud.init() != SoLoud::SO_NO_ERROR ) {
        return false;
    }

    m_Soloud.set3dSoundSpeed(DOD_AUDIO_SOUNDSPEED);

    m_Soloud.setGlobalVolume(0.0f);

    m_MusicBusHandle    = m_Soloud.play(m_MusicBus);
    m_AmbienceBusHandle = m_Soloud.play(m_AmbienceBus);
    m_SfxBusHandle      = m_Soloud.play(m_SfxBus);

    m_Soloud.fadeGlobalVolume(2.0f, 5.0); // fade in audio to avoid harsh sounds at startup

    CommandManager::Add("get_volume", GetVolume_f);
    CommandManager::Add("set_volume", SetVolume_f);

    return true;
}

void Audio::Deinit() {
    m_Soloud.deinit();
}

SoLoud::AudioSource* Audio::LoadSource(const std::string& path, float volume, bool loop, bool stream) {
    auto it = m_Sources.find(path);
    if ( it != m_Sources.end() ) {
        return it->second;
    } else {
        std::string filePath = g_GameDir + "audio/" + path;
        SoLoud::AudioSource* source;
        SoLoud::result result;
        if ( stream ) {
            SoLoud::WavStream* wavstream = new SoLoud::WavStream();
            result = wavstream->load(filePath.c_str());
            source = wavstream;
        } else {
            SoLoud::Wav* wav = new SoLoud::Wav();
            result = wav->load(filePath.c_str());
            source = wav;
        }
        if ( result != SoLoud::SO_NO_ERROR ) {
            printf("Loading audio source failed: %s (%s)\n", m_Soloud.getErrorString(result), path.c_str());
        }

        source->setVolume(volume);
        source->setLooping(loop);
        source->set3dAttenuation(DOD_AUDIO_ATTENUATION_MODEL, DOD_AUDIO_ATTENUATION_ROLLOFF);
        source->set3dMinMaxDistance(DOD_AUDIO_MIN_DISTANCE, DOD_AUDIO_MAX_DISTANCE);

        m_Sources.insert({ path, source });
        return source;
    }
}
