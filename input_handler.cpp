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
    
    return nullptr;
}

