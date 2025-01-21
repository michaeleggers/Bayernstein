#ifndef DYNAMICSOUND_H
#define DYNAMICSOUND_H

#include <string>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"

#include "soloud.h"

/**
 * An audio source with dynamic length, compounded out of multiple sounds.
 * The composite sound is played through the sfx audio bus.
 *
 * Inpired by https://youtu.be/MEizUamJTyk
 */
class DynamicSound {
  public:
    DynamicSound(SoLoud::AudioSource* loop, SoLoud::AudioSource* end);

    /** Start playing the sound. Loops until `End()` is called. */
    void Begin(double loopFadeIn = 0.5);
    /** Start playing the 3d sound at the given position. Loops until `End3d()` is called. */
    void Begin3d(glm::vec3 pos, glm::vec3 vel = glm::vec3(0.0f), double loopFadeIn = 0.5);
    /** Stop the currently playing sound, playing the additional end sound effect. */
    void End(double loopFadeOut = 0.5);
    /** Stop the currently playing sound, playing the additional end sound effect at the given position. */
    void End3d(glm::vec3 pos, glm::vec3 vel = glm::vec3(0.0f), double loopFadeOut = 0.5);

    // TODO: add support for updating the audio source position
    // TODO: add support for randomized variations (filters, sound snippets, etc.)
    // TODO: add support for sound instance (not audio source) dependent parameters (e.g. attenuation settings, or playback speed for sliding doors)?

  private:
    SoLoud::AudioSource* m_LoopSound;
    SoLoud::handle       m_LoopHandle = 0;

    SoLoud::AudioSource* m_EndSound;
};

#endif // DYNAMICSOUND_H
