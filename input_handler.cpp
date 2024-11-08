#include "input_handler.h"

#include <SDL.h>

// 'Low level' raw (SDL) input
#include "input.h"

InputHandler* InputHandler::Instance() {
    static InputHandler m_InputHandler;

    return &m_InputHandler;
}

void InputHandler::BindInputToActionName(int key, const std::string& actionName) {
    m_ActionNameToInput.insert({ actionName, key }); 
}

ButtonState InputHandler::GetMappedButtonState(const std::string& actionName) {
    // Check if a mapping exists for this action 
    const auto& actionToInput = m_ActionNameToInput.find(actionName);
    if ( actionToInput == m_ActionNameToInput.end() ) {
        return ButtonState::NONE;
    }

    // OK. Action exists. Check its button state.
    int key = actionToInput->second;

    if ( KeyPressed(key) ) {
        return ButtonState::PRESSED;
    }
    if ( KeyWentDown(key) ) {
        return ButtonState::WENT_DOWN;
    }
    if ( KeyWentUp(key) ) {
        return ButtonState::WENT_UP;
    }

    if ( MouseWentDown(key) ) {
        return ButtonState::WENT_DOWN;
    }
    if ( MouseWentUp(key) ) {
        return ButtonState::WENT_UP;
    }
    if ( MousePressed(key) ) {
        return ButtonState::PRESSED;
    }

    // Action exists but no button is active in any way.
    return ButtonState::NONE;
}

