#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include "command.h"

#include <unordered_map>
#include <string>

enum class ButtonState {
    NONE,
    WENT_DOWN,
    WENT_UP,
    PRESSED
};

class InputHandler {

public:
    // Singleton class!
    static InputHandler* Instance();

    Command*    HandleInput();
    void        BindInputToActionName(int key, const std::string& actionName);
    ButtonState GetMappedButtonState(const std::string& actionName); 

private:
    InputHandler()  = default;
    ~InputHandler() = default;

    std::unordered_map<std::string, int>         m_ActionNameToInput;
    
};

#define CHECK_ACTION(actionName) (InputHandler::Instance()->GetMappedButtonState(actionName))

#endif

