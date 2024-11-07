#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "command.h"

class JumpCmd : public Command {
public:
    static JumpCmd* Instance();

    void Execute() override;

private:
    JumpCmd() = default;
    ~JumpCmd() = default;
};

class UseCmd : public Command {
public:
    static UseCmd* Instance();

    void Execute() override;

private:
    UseCmd() = default;
    ~UseCmd() = default;
};

#endif

