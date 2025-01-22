#include "CommandManager.h"

#include <sstream>

#include "Console.h"
#include "VariableManager.h"

void Help_f(std::vector<std::string> args) {
    Console::Print("Use the console to show/change variables or run commands:");
    Console::Print("  - List available variables with 'list_vars', show it by typing the name, change it by passing the new value after it.");
    Console::Print("  - List available commands with 'list_cmds', command specific help is not yet available.");
}

void ListCmds_f(std::vector<std::string> args) {
    if (args.size() > 2) {
        Console::Print("Invalid number of arguments to list commands!");
    } else {
        std::string fmt = args.size() == 1 ? "All registered commands:" : "Registered commands matching '%s':";
        Console::Printf(fmt, args[ 1 ]);
        auto cmds = CommandManager::GetAll();
        for (const auto& cmd : cmds) {
            if (args.size() == 1 || cmd->name.find(args[ 1 ]) != std::string::npos) {
                Console::Printf("    %s", cmd->name);
            }
        }
    }
}

std::map<std::string, ConsoleCommand*> CommandManager::commands;

std::vector<std::string> CommandManager::TokenizeString(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream       stream(input);
    std::string              token;
    while ( stream >> token ) { // split at spaces
        tokens.push_back(token);
    }
    return tokens;
}

void CommandManager::Init() {
    Add("help", Help_f);
    Add("list_cmds", ListCmds_f);
}

ConsoleCommand* CommandManager::Add(std::string name, CommandHandler function) {
    ConsoleCommand* cmd = Find(name);
    if ( cmd ) {
        return cmd;
    } else if ( VariableManager::Find(name) ) {
        Console::Printf("Can't add command, %s is a variable.", cmd->name);
        return nullptr;
    }
    cmd = new ConsoleCommand(name, function);
    commands.insert({ name, cmd });
    return cmd;
}

ConsoleCommand* CommandManager::Find(std::string name) {
    auto it = commands.find(name);
    return it != commands.end() ? it->second : nullptr;
}

bool CommandManager::Exists(std::string name) {
    return Find(name) != nullptr;
}

std::vector<ConsoleCommand*> CommandManager::GetAll()
{
    std::vector<ConsoleCommand*> cmds;
    cmds.reserve(commands.size());
    for (const auto& [name, cmd] : commands) {
        cmds.push_back(cmd);
    }
    return cmds;
}


void CommandManager::ExecuteString(std::string input) {
    std::vector<std::string> args = TokenizeString(input);
    if ( args.size() > 0 ) {
        ConsoleCommand* cmd = Find(args[ 0 ]);
        if ( cmd ) {
            cmd->function(args);
        } else if ( !VariableManager::HandleCommand(args) ) {
            Console::Printf("Command '%s' not found.", args[ 0 ]);
        }
    }
}
