#include "command.h"

#include <stdio.h>


Command* Command::Instance() {
    static Command m_Command;

    return &m_Command;
}

void Command::execute() {
    printf("Test command. Remove later.\n");
}

