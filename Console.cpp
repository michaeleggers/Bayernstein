
#include "Console.h"

Console::Console(int lineBufferSize, int inputHistorySize) :
    m_lineBuffer(lineBufferSize), m_inputHistory(inputHistorySize) {}

int Console::ScrollPos() {
    return m_scrollPos;
}

void Console::Scroll(int delta) {
    m_scrollPos += delta;

    if (m_scrollPos >= m_lineBuffer.Size()) {
        m_scrollPos = m_lineBuffer.Size() - 1;
    } else if (m_scrollPos < 0) {
        m_scrollPos = 0;
    }
}

int Console::CursorPos() {
    return m_cursorPos;
}

void Console::MoveCursor(int delta) {
    m_cursorPos += delta;

    if (m_cursorPos > (int)m_currentInput.length()) {
        m_cursorPos = m_currentInput.length();
    } else if (m_cursorPos < 0) {
        m_cursorPos = 0;
    }
    m_blinkTimer = 0;
}

std::string Console::CurrentInput() {
    return m_currentInput;
}

void Console::SetInput(std::string input) {
    m_currentInput = input;
    m_cursorPos = input.length();
    m_blinkTimer = 0;
}

void Console::SetInputFromHistory(int delta) {
    if (delta == 0 || m_inputHistory.Size() == 0) return;
    m_inputHistoryPos += delta;

    if (m_inputHistoryPos >= m_inputHistory.Size()) {
        m_inputHistoryPos = m_inputHistory.Size() - 1;
    } else if (m_inputHistoryPos < -1) {
        m_inputHistoryPos = -1;
    }
    std::string str = "";
    if (m_inputHistoryPos > -1) {
        m_inputHistory.Get(m_inputHistoryPos, &str);
    }
    SetInput(str);
}

void Console::UpdateInput(std::string substr) {
    m_currentInput.insert(m_cursorPos, substr);
    m_cursorPos += substr.length();
    m_blinkTimer = 0;
}

void Console::DeleteInput(int delta) {
    if (delta < 0) {
        int prevCursorPos = m_cursorPos;
        MoveCursor(delta);
        int eraseCount = prevCursorPos - m_cursorPos;
        m_currentInput.erase(m_cursorPos, eraseCount);
    } else {
        m_currentInput.erase(m_cursorPos, abs(delta));
    }
    m_blinkTimer = 0;
}

void Console::SubmitInput() {
    m_lineBuffer.Push(m_currentInput);
    m_inputHistory.Push(m_currentInput);
    m_currentInput = "";
    m_inputHistoryPos = -1;
    m_cursorPos = 0;
    m_blinkTimer = 0;
    // TODO: parse/dispatch submitted commands
}
