#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>

#include "CircularBuffer.h"

/**
 * Backend for the in-game console.
 * Mainly responsible for handling/storing user (command) input, as well as the command and log history.
 * Implemented as a singleton for printing support, use `Console::Create` instead of the constructor.
 */
class Console {
private:
    /** The current (editable) input string. */
    std::string m_currentInput = "";
    /** Current position in the history to use for replacing the current input. `-1` indicates current/empty input. */
    int m_inputHistoryPos = -1;
    /** Typing cursor position (column offset). */
    int m_cursorPos = 0;
    /** Scroll position of the console log (line offset). */
    int m_scrollPos = 0;

private:
    /** The singleton instance of the console. */
    static Console* instance;
    /** Create/initialize the console and its buffers, etc. */
    Console(int lineBufferSize, int inputHistorySize);

public:
    /** Buffer for storing the lines written to the console. */
    CircularBuffer m_lineBuffer;
    /** Buffer for storing the executed command/input history. */
    CircularBuffer m_inputHistory;
    /** Flag to indicate if the console is active (i.e. visible on screen). */
    bool m_isActive = false;
    /** Milliseconds since last cursor blink cycle. */
    double m_blinkTimer = 0;

public:
    Console(const Console &obj) = delete; // delete copy constructor

    /** Create the console (singleton) instance and initialize it and its buffers, etc. */
    static Console* Create(int lineBufferSize, int inputHistorySize);

    /** Get the current scroll position (line offset) of the console log. */
    int ScrollPos();
    /** Move the scroll position by the given delta (limited by the line buffer size). */
    void Scroll(int delta);
    /** Get the current input cursor position (column offset). */
    int CursorPos();
    /** Move the input cursor position by the given delta (limited to the input length). */
    void MoveCursor(int delta);
    /** Get the current (editable) input string. */
    std::string CurrentInput();
    /** Replace the current input with the given text. */
    void SetInput(std::string input);
    /** Replace the current input with a previous value from the history, relative to the previously used history entry. */
    void SetInputFromHistory(int delta);
    /** Update the current input by inserting the given substring at the cursor position. */
    void UpdateInput(std::string substr);
    /** Delete the given number of characters from the current input, starting from the cursor position (limited to the input length). */
    void DeleteInput(int delta);
    /** Confirm and submit the current input (usually on line-feed). */
    void SubmitInput();

public:
    /** Prints the given string to the console as a line. */
    static void Print(std::string str);
    /** Formats and prints the given string to the console as a line. */
    template<typename... Args>
    static void Printf(std::string fmt, Args const&... args) { // implementation must be in header due to variadic template
        PrintfImpl(fmt.c_str(), convertArg(args)...);
    }

private:
    /** C-style implementation for formatting the given string and printing it to the console. */
    static void PrintfImpl(const char* fmt, ...);
    /** Helper function to convert the variadic argument list to be printf compatible. */
    template<typename T>
    static decltype(auto) convertArg(T const& arg) { return arg; }
    /** Helper function to convert the variadic argument list to be printf compatible. `std::string` is converted to c strings. */
    static const char* convertArg(std::string const& arg) { return arg.c_str(); }
};

#endif // CONSOLE_H
