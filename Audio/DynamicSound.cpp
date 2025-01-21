#include "DynamicSound.h"

#include "Audio.h"

DynamicSound::DynamicSound(SoLoud::AudioSource* loop, SoLoud::AudioSource* end) {
    m_LoopSound = loop;
    m_EndSound  = end;
}

void DynamicSound::Begin(double loopFadeIn) {
    if ( Audio::m_Soloud.isValidVoiceHandle(m_LoopHandle) ) {
        return; // already running
    } else if ( loopFadeIn > 0.0 ) {
        m_LoopHandle = Audio::m_SfxBus.play(*m_LoopSound, 0.0f);
        Audio::m_Soloud.fadeVolume(m_LoopHandle, m_LoopSound->mVolume, loopFadeIn);
    } else {
        m_LoopHandle = Audio::m_SfxBus.play(*m_LoopSound, -1);
    }
    // TODO: ensure the audio instance is set to loop (also in Begin3d)?
}

void DynamicSound::Begin3d(glm::vec3 pos, glm::vec3 vel, double loopFadeIn) {
    if ( Audio::m_Soloud.isValidVoiceHandle(m_LoopHandle) ) {
        return; // already running
    } else if ( loopFadeIn > 0.0 ) {
        m_LoopHandle = Audio::m_SfxBus.play3d(*m_LoopSound, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, 0.0f);
        Audio::m_Soloud.fadeVolume(m_LoopHandle, m_LoopSound->mVolume, loopFadeIn);
    } else {
        m_LoopHandle = Audio::m_SfxBus.play3d(*m_LoopSound, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
    }
}

void DynamicSound::End(double loopFadeOut) {
    if ( !Audio::m_Soloud.isValidVoiceHandle(m_LoopHandle) ) return; // not running

    Audio::m_SfxBus.play(*m_EndSound, -1);
    if ( loopFadeOut > 0 ) {
        Audio::m_Soloud.fadeVolume(m_LoopHandle, 0.0, loopFadeOut);
        Audio::m_Soloud.scheduleStop(m_LoopHandle, loopFadeOut);
    } else {
        Audio::m_Soloud.stop(m_LoopHandle);
    }
}

void DynamicSound::End3d(glm::vec3 pos, glm::vec3 vel, double loopFadeOut) {
    if ( !Audio::m_Soloud.isValidVoiceHandle(m_LoopHandle) ) return; // not running

    Audio::m_SfxBus.play3d(*m_EndSound, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, -1);
    if ( loopFadeOut > 0 ) {
        Audio::m_Soloud.fadeVolume(m_LoopHandle, 0.0, loopFadeOut);
        Audio::m_Soloud.scheduleStop(m_LoopHandle, loopFadeOut);
    } else {
        Audio::m_Soloud.stop(m_LoopHandle);
    }
}
