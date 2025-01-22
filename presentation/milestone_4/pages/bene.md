---
layout: center
---

# Gegner KI

---
hideInToc: true
---

# Überblick
- **Dungeons of Danger** nutzt ein **modulares KI-System**, das Gegnern dynamisches und glaubwürdiges Verhalten verleiht.
- **Finite State Machines (FSMs)** bilden das Herzstück:
  - Zustände wie Idle, Attack, etc.
- Ein **zentraler Nachrichtenaustausch** (MessageDispatcher + Telegram) ermöglicht eine schnelle Kommunikation zwischen Gegnern.

---
hideInToc: true
---

# Kernelemente
- **Steering Behaviors** (z. B. Seek, Flee, Pursuit, Evade):
  - Sorgen für natürliche, flüssige Bewegungen.
  - Kombination mehrerer Kräfte (Weighted Truncated Sum).
- **Sensing**

---
hideInToc: true
---

# Unser Wahrnehmungs Modell

**Was nimmt die Entity wahr?**
- **Distanz**: Wie weit ist ein Objekt entfernt?  
- Die **Z-Position** des Objekts im Raum.  
- **Winkel**: Relative Ausrichtung des Objekts zur Sichtlinie.

---
hideInToc: true
---
![Alt text](/img/bene/cake.png){width=90% height=90%}

---
hideInToc: true
---
# Weitere Sinne
- Tast-Sinn über Kollisionserkennung
- Allgmeines Wohlbefinden über Lebenspunkte

später noch: **Verdeckung** durch Level Geometrie

---
hideInToc: true
---

# Ausblick
- **Erweiterbarkeit**: Leichtes Hinzufügen weiterer Zustände & Verhaltensweisen.
- **Realistischeres Sensing**:
  - Einbezug von Level-Geometrie (Wände, Hindernisse).
  - Verfeinerung von Kollisionsabfragen.
- **Mögliche Zusatzideen**:
  - Team-Verhalten (z. B. Boids).
  - Weitere Sinne (z. B. Hören, Riechen).
