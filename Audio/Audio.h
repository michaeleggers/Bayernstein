#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <unordered_map>

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

    /**
     * Creates an audio source from the given file.
     * If the file was already loaded, a reference to the existing source is returned (and given defaults are ignored).
     * @param path the audio file to load, relative to the 'audio' asset directory
     * @param volume default volume for audio instances
     * @param loop default flag if the audio instances should repeat
     * @param stream stream from disk during playback if `true`, load audio into memory otherwise
     */
    static SoLoud::AudioSource* LoadSource(const std::string& path, float volume = 1.0f, bool loop = false, bool stream = false);

    Audio() = delete; // don't allow instantiation

  private:
    static std::unordered_map<std::string, SoLoud::AudioSource*> m_Sources;
};

#endif // AUDIO_H
