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
        if ( m_Enabled ) {
            m_InputReceiver->HandleInput();
        }
    }
}

void CInputDelegate::Enable() {
    m_Enabled = true;
}

void CInputDelegate::Disable() {
    m_Enabled = false;
}
