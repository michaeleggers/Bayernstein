#ifndef _INPUT_DELEGATE_H_
#define _INPUT_DELEGATE_H_

#include "input_receiver.h"

class CInputDelegate {

  public:
    static CInputDelegate* Instance();

    void SetReceiver(IInputReceiver* receiver);
    void HandleInput();
    /** Allow to disable this higher-level input system so that
     *  only raw OS input is considered as input. This is
     *  useful to not interfere with the console.
    */
    void Enable();
    void Disable();

  private:
    /* Singleton! */
    CInputDelegate()  = default;
    ~CInputDelegate() = default;

    IInputReceiver* m_InputReceiver = nullptr;
    bool            m_Enabled       = true;
};

#endif
