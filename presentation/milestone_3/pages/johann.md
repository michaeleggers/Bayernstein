---
layout: center
---

# Audio 

---
hideInToc: true
---

# Audio (Management)
<br>
Konfigurierbares Mixing über mehrere Audio-Busse (über Konsole änderbar):
```cpp
Audio::m_SfxBus
Audio::m_AmbienceBus
Audio::m_MusicBus
```

Globales `AudioSource` Management:
```cpp
Audio::LoadSource("sfx/footsteps.wav");
```
Wird einmalig in Memory geladen, dann von mehreren Entities/Entity-Instanzen referenzierbar.

<!--
- Konsole:
    - `get_volume` zum Anzeigen der Bus-Lautstärken
    - `set_volume <bus name> 0.5` zum Ändern der Bus-Lautstärke
    - `set_volume 0.5` == `set_volume global 0.5`
    - Werte > 1 erlaubt (zu hohe Werte führen ggf. zu distortion)

- LoadSource setzt auch paar sinnvolle defaults.
- eine `AudioSource` repräsentiert das Sound-File selbst (ist ggf. in-memory), um den Sound abzuspielen kann jede Entity etc. eine eigene Play-Instanz davon erzeugen.
-->

---
hideInToc: true
---

# Audio (2D Sounds)
- einfach zu verwenden
- für Musik, Ambience und Player SFX
```cpp
Audio::m_AmbienceBus.play(*m_Ambience, -1);
```

```cpp
if ( m_PrevCollisionState == ES_ON_GROUND ) {
    if ( m_WantsToJump ) {
        Audio::m_SfxBus.play(*m_SfxJump, -1);
        printf("Jumping....\n");
    }
}
```

<!--
- (Der Parameter `-1` forced default Volume (in der AudioSource beim Laden festgelegt), ist ein Bug bei playback in einem Bus)
-->


---
hideInToc: true
---

# Audio (3D Sounds)
- für Environment und Entity SFX

- die Audio-Engine (bzw. diverse Effekte) muss mit korrekter Skalierung konfiguriert werden:
```cpp
constexpr float DOD_COORD_UNIT_FACTOR = 37.65f; // derived from quake units
```

- Attenuation bestimmt, wie Sounds mit Entfernung leiser werden:
```cpp
audioSource.set3dAttenuation(DOD_AUDIO_ATT_MODEL, DOD_AUDIO_ATT_ROLLOFF);
audioSource.set3dMinMaxDistance(DOD_AUDIO_MIN_DIST, DOD_AUDIO_MAX_DIST);
```

<!--
- 3D Audio Beispiele: Enemy Sounds (z.B. Footsteps), Türen, etc.

- durch die Konfiguration der Skalierung kann man der Audio-Engine direkt die in-game Koordinaten geben
- Notwendig für z.B. SoundSpeed (für Doppler-Effekt), oder Attenuation
- Faktor ist die Umwandlung von Meter zu Welt (Quake) Einheiten.

- unser Attenuation model (momentan): INVERSE_DISTANCE
- mit den Settings kann man auch noch weiter rumprobieren

- Variablen Namen sind für die Folien-Darstellung gekürzt
-->

---
hideInToc: true
---

# Audio (3D Sounds)

- Sound wird als 3D abgespielt, die Position muss ggf. kontinuierlich aktualisiert werden:
```cpp
if ( Audio::m_Soloud.isValidVoiceHandle(m_SfxHandle) ) {
  Audio::m_Soloud.set3dSourcePosition(m_SfxHandle, pos.x, pos.y, pos.z);
  Audio::m_Soloud.set3dSourceVelocity(m_SfxHandle, vel.x, vel.y, vel.z);
  Audio::m_Soloud.update3dAudio();
} else {
  m_SfxHandle =
    Audio::m_SfxBus.play3d(*m_Sfx, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z);
}
```

<!--
- Aktualisierung gilt für bewegiche Entities, v.a. für länger spielende Samples, hier z.B. Footsteps (ggf. nicht notwendig/erwünscht bei z.B. Enemy Gunshot)

- Variablen Namen sind für die Folien-Darstellung gekürzt
-->

---
hideInToc: true
---

# Audio (3D Sounds)
- Die Position des Listeners (Player) muss ebenfalls für die 3D-Berechnung aktualisiert werden:
```cpp
Audio::m_Soloud.set3dListenerPosition(position.x, position.y, position.z);
Audio::m_Soloud.set3dListenerAt(forward.x, forward.y, forward.z);
Audio::m_Soloud.set3dListenerVelocity(velocity.x, velocity.y, velocity.z);
Audio::m_Soloud.set3dListenerUp(WORLD_UP.x, WORLD_UP.y, WORLD_UP.z);
Audio::m_Soloud.update3dAudio();
```

<!--
- Wichtig ist, dass diese Aktualisierung nur in der im Input-Delegate aktiven Entity aktualisiert wird, dass die Position immer zum aktiven Player/Camera passt.

- Variablen Namen sind für die Folien-Darstellung gekürzt
-->

---
hideInToc: true
---

# Audio (Dynamische Sounds)
- Sounds mit unbestimmter Länge (z.B. Türen)
- wird durch Events aktiviert/deaktiviert
- zusammengesetzt aus Basis-Loop und einem  
Abschluss-Effekt am Ende
```cpp
class DynamicSound {
  public:
    DynamicSound(SoLoud::AudioSource* loop, SoLoud::AudioSource* end);
    void Begin3d(glm::vec3 pos, glm::vec3 vel, double loopFadeIn = 0.5);
    void End3d(glm::vec3 pos, glm::vec3 vel, double loopFadeOut = 0.5);
```
<img src="/img/johann/dynamic_sound_loop.png" style="right:15px;top:90px;position:absolute;width:calc(100% / 4.5);">

<!--
- Screenshot zeigt:
    - Sample in der Mitte zerschnitten und Hälften umgedreht
    - -> dadurch Überblendung von Sample-Anfang/Ende möglich
    - -> seamless loop

- Vorerst Basisimplementierung, wäre erweiterbar z.B. durch andere Randomizations (Effekte, Sound Snippets, etc.)

- Definitionen sind für die Folien-Darstellung gekürzt
-->

