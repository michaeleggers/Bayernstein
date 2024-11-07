#include "input_handler.h"

#include <SDL.h>

// 'Low level' raw (SDL) input
#include "input.h"
#include "commands.h"

InputHandler* InputHandler::Instance() {
    static InputHandler m_InputHandler;

    return &m_InputHandler;
}

Command* InputHandler::HandleInput() {
    if ( KeyPressed(SDLK_SPACE) ) {
        return JumpCmd::Instance();
    }
    if ( KeyPressed(SDLK_e) ) {
        return UseCmd::Instance();
    }
    if ( KeyPressed(SDLK_0) ) {

    }
    
    return nullptr;
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
    
    if ( KeyPressed(actionToInput->second) ) {
        return ButtonState::PRESSED;
    }

    if ( MouseWentDown(actionToInput->second) ) {
        return ButtonState::WENT_DOWN;
    }

    if ( MouseWentUp(actionToInput->second) ) {
        return ButtonState::WENT_UP;
    }
    
    if ( MousePressed(actionToInput->second) ) {
        return ButtonState::PRESSED;
    }

    // Action exists but no button is active in any way.
    return ButtonState::NONE;
}

