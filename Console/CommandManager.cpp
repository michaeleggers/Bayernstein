#include "CommandManager.h"

#include <sstream>

#include "VariableManager.h"
#include "Console.h"

std::map<std::string, ConsoleCommand*> CommandManager::commands;

std::vector<std::string> CommandManager::TokenizeString(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    while (stream >> token) { // split at spaces
        tokens.push_back(token);
    }
    return tokens;
}

ConsoleCommand* CommandManager::Add(std::string name, CommandHandler function) {
    ConsoleCommand* cmd = Find(name);
    if (cmd) {
        return cmd;
    } else if (VariableManager::Find(name)) {
        Console::Printf("Can't add command, %s is a variable.", cmd->name);
        return nullptr;
    }
    cmd = new ConsoleCommand(name, function);
    commands.insert({name, cmd});
    return cmd;
}

ConsoleCommand* CommandManager::Find(std::string name) {
    auto it = commands.find(name);
    return it != commands.end() ? it->second : nullptr;
}

bool CommandManager::Exists(std::string name) {
    return Find(name) != nullptr;
}

void CommandManager::ExecuteString(std::string input) {
    std::vector<std::string> args = TokenizeString(input);
    if (args.size() > 0) {
        ConsoleCommand* cmd = Find(args[0]);
        if (cmd) {
            cmd->function(args);
        } else if (!VariableManager::HandleCommand(args)) {
            Console::Printf("Command '%s' not found.", args[0]);
        }
    }
}
