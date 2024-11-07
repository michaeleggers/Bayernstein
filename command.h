#ifndef _COMMAND_H_
#define _COMMAND_H_

class Command {

public:

    Command() = default;
    ~Command() = default;

    virtual void Execute() = 0;

private:

};


#endif

