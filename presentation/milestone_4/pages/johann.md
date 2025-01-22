---
layout: center
---

# In-Game Console

---
hideInToc: true
---

# In-Game Console (Recap)
- User-Input mit bewegbarem Cursor, Löschen und Entfernen, an der Cursor-Position Zeichen einfügen
- formatiertes Printing
- Scrolling des Logs
- Befehls-Historie
- blinkender Cursor
- Screen-Overflow Behandlung
- Variablen- und Befehlsmanagement

<img src="/img/johann/Console_Vizualization.png" style="position: absolute; bottom: 45px; right: 0; width: 50%;">

---
hideInToc: true
---

# In-Game Console (Neue Befehle)

<img src="/img/johann/command_help.png" style="width: 100%;">
<div style="display: flex; gap: 80px; width: 100%;margin: 10px 0;">
  <img src="/img/johann/command_list_vars.png" style="flex: 1;width: 30%;margin-left: 60px;">
  <img src="/img/johann/command_list_cmds.png" style="flex: 1;width: 30%;margin-right: 60px;">
</div>
Die Auflistung kann durch Übergabe eines Substring als Befehlsparameter gefiltert werden.

---
layout: center
---

# Audio 

---
hideInToc: true
---

# Audio (Recap)
<div></div>
Zentralisiertes Engine- und Ressource-Management:
```cpp
Audio::m_SfxBus
Audio::m_AmbienceBus
Audio::m_MusicBus
Audio::m_Soloud
Audio::LoadSource(...)
```
2D, 3D und dynamische Sounds:
```cpp
m_MusicHandle = Audio::m_MusicBus.play(*m_Soundtrack, -1);
m_SfxHandle =
  Audio::m_SfxBus.play3d(*m_Sfx, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z);
m_SfxMovement = new DynamicSound(sfxLoop, sfxEnd);
```


---
hideInToc: true
---

# Audio und HUD (Weapon)

```cpp
bool Weapon::Fire() {
  if ( m_TimeElapsed >= m_FireRate && m_RoundsRemaining > 0 ) {
      --m_RoundsRemaining;
      m_TimeElapsed = 0.0;
      Audio::m_SfxBus.play(*m_SfxGunshot, -1);
      if ( m_RoundsRemaining == 0 ) m_TimeElapsed -= m_ReloadTime;
      return true;
  }
  return false;
}
```
Reload wird im Update-Loop geprüft und zu passendem Zeitpunkt ausgeführt.

---
hideInToc: true
---

# Audio und HUD (Weapon)

<img src="/img/johann/HUD.png" style="width: 70%;margin: 0 auto;">

---
hideInToc: true
---

# Boot Screen (Bonus)

<img src="/img/johann/loading_screen.png" style="margin:-20px auto 0 auto;width:88%;">