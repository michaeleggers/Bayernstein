#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include "command.h"

class InputHandler {

public:
    static InputHandler* Instance();

    Command* HandleInput();

private:
    InputHandler() = default;
    ~InputHandler() = default;
    
};

#endif

