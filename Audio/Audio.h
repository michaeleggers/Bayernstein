#ifndef AUDIO_H
#define AUDIO_H

#include "soloud.h"
#include "soloud_bus.h"

/**
 * The (global) management for game audio.
 * Provides access to the sound engine and various mixing buses.
 * 
 * NOTE:
 * When working with voice handles (`SoLoud::handle`) that can be invalid/unused, make sure to properly initialize them with `0`!
 * Otherwise they might have a random value that matches a different sound (or mixing bus) handle, causing that to get messed up unexpectedly.
 */
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
