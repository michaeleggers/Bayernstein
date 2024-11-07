#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <map>
#include <string>
#include <vector>

typedef void (*CommandHandler) (std::vector<std::string> args);

struct ConsoleCommand {
    const std::string name;
    CommandHandler function;
    // TODO: consider adding description / help text and help command
    ConsoleCommand(std::string n, CommandHandler f) : name(n), function(f) {}
};

/**
 * Manages commands that are accessible from the console.
 * Loosely based on Quake's (QSS-M) `cmd` implementation.
 */
class CommandManager {
private:
    /** List of registered commands, mapped by their (unique) name. */
    static std::map<std::string, ConsoleCommand*> commands;

private:
    /** Splits the given string into space delimited tokens. */
    static std::vector<std::string> TokenizeString(const std::string& input);

public:
    /** Adds a command with the given name and the given callback to be available in the console. */
    static ConsoleCommand* Add(std::string name, CommandHandler function);
    // static void Remove(ConsoleCommand* cmd);
    // static bool RegisterAlias(ConsoleCommand* cmd, std::string alias);
    /** Finds the command with the given name in the registered commands. */
    static ConsoleCommand* Find(std::string name);
    /** Checks if a command with the given name is defined. */
    static bool Exists(std::string name);

    /** Parses the user input string into command arguments and executes the command's handler if available. */
    static void ExecuteString(std::string input);

    CommandManager() = delete; // don't allow instantiation
};

#endif // COMMANDMANAGER_H
