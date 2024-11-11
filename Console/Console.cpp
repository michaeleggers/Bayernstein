#include "Console.h"

#include <stdarg.h>

#include <SDL.h>

#include "CommandManager.h"
#include "VariableManager.h"

#include "../utils.h"
#include "../irender.h"
#include "../r_font.h"
#include "../hkd_interface.h"
#include "../input.h"


ConsoleVariable con_stdout = {"con_stdout", 1};


Console* Console::instance = nullptr;

Console::Console(int lineBufferSize, int inputHistorySize) :
    m_lineBuffer(lineBufferSize), 
    m_inputHistory(inputHistorySize) {

        m_pConsoleFont = new CFont("fonts/HackNerdFont-Bold.ttf", 26);
        IRender *renderer = GetRenderer();
        renderer->RegisterFont(m_pConsoleFont);
}

Console* Console::Create(int lineBufferSize, int inputHistorySize) {
    if (instance == nullptr) {
        instance = new Console(lineBufferSize, inputHistorySize);
        VariableManager::Register(&con_stdout);
    }
    return instance;
}

const Console* Console::Instance() {
    assert( instance != nullptr );
    
    return instance;
}

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
    Print(m_currentInput);
    m_inputHistoryPos = -1;
    m_cursorPos = 0;
    m_scrollPos = 0;
    m_blinkTimer = 0;

    if (m_currentInput == "") return;
    std::string lastInput;
    if (!m_inputHistory.Get(0, &lastInput) || lastInput != m_currentInput) {
        m_inputHistory.Push(m_currentInput);
    }
    CommandManager::ExecuteString(m_currentInput);
    m_currentInput = "";
}

void Console::Print(std::string str) {
    if (con_stdout.value) {
        printf("%s\n", str.c_str());
    }
    instance->m_lineBuffer.Push(str);
}

void Console::PrintfImpl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list argsCpy;
    va_copy(argsCpy, args);
    char buffer[ vsnprintf(NULL, 0, fmt, args) + 1 ];
    va_end(args);
    vsnprintf(buffer, sizeof(buffer), fmt, argsCpy); 
    va_end(argsCpy);

    if (con_stdout.value) {
        printf("%s\n", buffer);
    }
    instance->m_lineBuffer.Push(std::string(buffer));
}

void Console::Run() {
    float msPerFrame = (float)GetDeltaTime();
    m_blinkTimer += msPerFrame;
    if (TextInput().length()) {
        UpdateInput(TextInput());
    } else if (KeyWentDownUnbuffered(SDLK_UP)) { // TODO: add support for 'repeat' mode
        SetInputFromHistory(1);
    } else if (KeyWentDownUnbuffered(SDLK_DOWN)) {
        SetInputFromHistory(-1);
    } else if (KeyWentDownUnbuffered(SDLK_LEFT)) {
        MoveCursor(-1);
    } else if (KeyWentDownUnbuffered(SDLK_RIGHT)) {
        MoveCursor(1);
    } else if (KeyWentDownUnbuffered(SDLK_BACKSPACE)) {
        DeleteInput(-1);
    } else if (KeyWentDownUnbuffered(SDLK_DELETE)) {
        DeleteInput(1);
    } else if (KeyWentDown(SDLK_RETURN)) {
        SubmitInput();
    }
    
    IRender* renderer = GetRenderer();
    //renderer->RenderBegin();
    renderer->RenderConsole(this, m_pConsoleFont);
    //renderer->RenderEnd();
}

