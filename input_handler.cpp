#include "input_handler.h"

#include <SDL.h>

// 'Low level' raw (SDL) input
#include "input.h"

InputHandler* InputHandler::Instance() {
    static InputHandler m_InputHandler;

    return &m_InputHandler;
}

Command* InputHandler::HandleInput() {
    if ( KeyPressed(SDLK_t) ) {
        return Command::Instance();
    }
    
    return nullptr;
}

