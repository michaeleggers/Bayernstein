# Überblick
- Input System
- In-Game-Console
- Lightmaps
- Türen reagieren korrekt auf Entities
- Pfade

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
CInputDelegate::Instance()->SetReceiver(m_pPlayerEntity);
...

```

#### Entity, die Input abfragen kann implementiert Input Receiver Interface...
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

# 



---

# Entity System

### Im Moment:
- Eine Base-class
- Entities leiten alle von Base-class ab: Player, Enemy, FlyCamera, FollowCamera, ...

Problem:
- Properties bisher alle in konkreten Entity-Klassen.
- Oftmals benötigen andere Entities Daten einer Entity.
- Resultat: Properties wandern in die Base-class.
