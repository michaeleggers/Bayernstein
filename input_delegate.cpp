#include "input_delegate.h"

#include "input_receiver.h"

CInputDelegate* CInputDelegate::Instance() {
    static CInputDelegate m_InputDelegate;

    return &m_InputDelegate;
}

void CInputDelegate::SetReceiver(IInputReceiver* receiver) {
    m_InputReceiver = receiver;
}

void CInputDelegate::HandleInput() {
    if ( m_InputReceiver != nullptr ) {
        m_InputReceiver->HandleInput();
    }
}




