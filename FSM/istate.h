//
// Created by benek on 10/14/24.
//

#ifndef STATE_H
#define STATE_H
#include "../Message/telegram.h"

template <class entity_type> class State {
  public:
    virtual ~State() = default;

    // this will execute when the state is entered
    virtual void Enter(entity_type*) = 0;

    // this is the states normal update function
    virtual void Execute(entity_type*) = 0;

    // this will execute when the state is exited.
    virtual void Exit(entity_type*) = 0;

    // this executes if the agent receives a message from the
    // message dispatcher
    virtual bool OnMessage(entity_type*, const Telegram&) = 0;
};

#endif // STATE_H
