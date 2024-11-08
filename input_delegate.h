#ifndef _INPUT_DELEGATE_H_
#define _INPUT_DELEGATE_H_

#include "input_receiver.h"

class CInputDelegate {

public:
    static CInputDelegate* Instance();

    void SetReceiver(IInputReceiver* receiver);
    void HandleInput();

private:
    /* Singleton! */
    CInputDelegate()  = default;
    ~CInputDelegate() = default;

    IInputReceiver* m_InputReceiver = nullptr;
};

#endif

