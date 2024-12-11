#ifndef AUDIO_H
#define AUDIO_H

#include "soloud.h"
#include "soloud_bus.h"

class Audio {
  public:
    static SoLoud::Soloud m_Soloud;

    static SoLoud::Bus    m_MusicBus;
    static SoLoud::handle m_MusicBusHandle;

    static SoLoud::Bus    m_AmbienceBus;
    static SoLoud::handle m_AmbienceBusHandle;
    
    static SoLoud::Bus    m_SfxBus;
    static SoLoud::handle m_SfxBusHandle;

  public:
    static bool Init();
    static void Deinit();

    Audio() = delete; // don't allow instantiation
};

#endif // AUDIO_H
