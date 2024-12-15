---
layout: center
---

# Was hat sich getan?

---
hideInToc: true
---
# Was hat sich getan?
- Input System
- In-Game-Console
- Lightmaps
- Türen reagieren korrekt auf Entities
- Pfade
- Bugfixes

---
hideInToc: true
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

