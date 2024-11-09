
#include "commands.h"

#include <stdio.h>

JumpCmd* JumpCmd::Instance() {
    static JumpCmd m_Cmd;
    
    return &m_Cmd;
}

void JumpCmd::Execute() {
    printf("Jumping\n");
}

UseCmd* UseCmd::Instance() {
    static UseCmd m_Cmd;
    
    return &m_Cmd;
}

void UseCmd::Execute() {
    printf("Using...\n");
}

