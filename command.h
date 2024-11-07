#ifndef _COMMAND_H_
#define _COMMAND_H_

class Command {

public:
    static Command* Instance();

    Command() = default;
    ~Command() = default;

    void execute();

private:

};

#endif

