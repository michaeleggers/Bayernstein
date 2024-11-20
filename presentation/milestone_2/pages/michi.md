# Was hat sich getan?
- Input System
- In-Game-Console
- Lightmaps
- Türen reagieren korrekt auf Entities
- Pfade
- Bugfixes

---

# Input System

#### Higher Level Game Code

```cpp
...
CInputHandler* inputHandler = CInputHandler::Instance();
inputHandler->BindInputToActionName(SDLK_SPACE, "jump");
inputHandler->BindInputToActionName(SDLK_0, "equip_rocketlauncher");
inputHandler->BindInputToActionName(SDLK_w, "forward");
...
// Let player entity respond to input events
CInputDelegate::Instance()->SetReceiver(m_pPlayerEntity);
...

```

#### Entity implementiert Input Receiver Interface...
```cpp
class Player : public BaseGameEntity, public IInputReceiver {
    ...
}
```

#### ...um auf Input zu prüfen:
```cpp
ButtonState jumpState = CHECK_ACTION("jump");
if (jumpState == ButtonState::PRESSED) {
    printf("I am jumping!\n");
}
```
---

# Entity System

### Im Moment:
- Eine Base-class
- Entities leiten alle von Base-class ab: Player, Enemy, FlyCamera, FollowCamera, ...

Problem:
- Properties bisher alle in konkreten Entity-Klassen.
- Oftmals benötigen andere Entities Daten einer Entity.
- Resultat: Properties wandern in die Base-class.

---

# Herausforderungen

- Collision Detection: Schlechte Performance. **Alle** Entities prüfen jedes Frame auf Kollision mit
**gesamter Welt**. Unterteilung in Octree würde Abhilfe schaffen.

- Collision Detection: Nicht framerate unabhängig.

- Animtationssystem: Schlechte Performance. Das Aufbauen der aktuellen Pose ist im Moment sehr teuer.
Wahrscheinlich schlechte Implementierung. Optimierung über Compute Shader wäre möglich.

- Entity System: Wir überlegen auf ein **Actor-Component** Modell wie in Unreal Engine umzustellen,
um den Code innerhalb einer Entity etwas aufzuräumen gleichartiges Verhalten syntaktisch zu komprimieren.

- Engine unterstützt ausschließlich TGA-Files für Welttexturen.

---

# Zu implementierende Features

- First Person Camera

- HUD Rendering für Lebensanzeige und Fadenkreuz

- Schießen auf Gegner

- Gegner AI weiter ausbauen (Navigation durch Welt, Interaktion mit Spieler)

- Eigene Gegner-Modelle

- Audio

- Lightmaps in Engine integrieren

- Memory Manager

- Virtuelles Filesystem integrieren: PhysicsFS (https://icculus.org/physfs/)






