#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include "command.h"

#include <unordered_map>
#include <string>

enum class ButtonState {
    NONE,
    WENT_DOWN,
    WENT_UP,
    PRESSED,
    MOVED
};

class CInputHandler {

public:
    // Singleton class!
    static CInputHandler* Instance();

    void        BindInputToActionName(int key, const std::string& actionName);
    ButtonState GetMappedButtonState(const std::string& actionName); 

private:
    CInputHandler()  = default;
    ~CInputHandler() = default;

    std::unordered_map<std::string, int>         m_ActionNameToInput;
    
};

#define CHECK_ACTION(actionName) (CInputHandler::Instance()->GetMappedButtonState(actionName))

#endif

