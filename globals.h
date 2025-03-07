#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "soloud.h"

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

constexpr glm::vec3 DOD_WORLD_UP(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 DOD_WORLD_FORWARD(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 DOD_WORLD_RIGHT(1.0f, 0.0f, 0.0f);

/**
 * Conversion factor from meteres to world (quake) units. Mainly needed for 3d audio effects.
 * @see https://quakewiki.org/wiki/unit
 */
constexpr float DOD_COORD_UNIT_FACTOR = 37.65f;

// speed of sound in air (m/s) converted to game units
constexpr float DOD_AUDIO_SOUNDSPEED = 343.0f * DOD_COORD_UNIT_FACTOR;

constexpr SoLoud::AudioSource::ATTENUATION_MODELS DOD_AUDIO_ATTENUATION_MODEL   = SoLoud::AudioSource::INVERSE_DISTANCE;
constexpr float                                   DOD_AUDIO_ATTENUATION_ROLLOFF = 0.1f;
constexpr float                                   DOD_AUDIO_MIN_DISTANCE        = 1.0f * DOD_COORD_UNIT_FACTOR;
constexpr float                                   DOD_AUDIO_MAX_DISTANCE        = 1000000.0f * DOD_COORD_UNIT_FACTOR;

// Update collisions at 60Hz tickrate.
constexpr double DOD_FIXED_UPDATE_TIME = 1000.0 / 60.0;

// What is considered to be a pretty short distance for the
// collision test in 'collision.cpp'
// NOTE: This distance depends very much on the size of the level geometry!
constexpr float DOD_VERY_CLOSE_DIST = 0.01f; // TODO: Collision with double precision?

constexpr size_t      DOD_SUPPORTED_IMAGE_EXTENSION_COUNT = 3;
constexpr const char* DOD_IMAGE_EXTENSION_NAMES[ DOD_SUPPORTED_IMAGE_EXTENSION_COUNT ] = { ".jpg", ".png", ".tga" };

#endif
