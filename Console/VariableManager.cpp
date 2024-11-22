#include "VariableManager.h"

#include <stdlib.h>

#include "CommandManager.h"
#include "Console.h"
#include "../utils/utils.h"


void Reset_f(std::vector<std::string> args) {
    if (args.size() == 2) {
        VariableManager::Reset(args[1]);
    } else {
        Console::Print("Invalid number of arguments to reset variable!");
    }
}


std::map<std::string, ConsoleVariable*> VariableManager::m_Variables;

void VariableManager::Init() {
    CommandManager::Add("reset", Reset_f);
}

bool VariableManager::Register(ConsoleVariable* var) {
    if (CommandManager::Find(var->name)) {
        Console::Printf("Can't register variable, '%s' is a command.", var->name);
        return false;
    }

    bool inserted = m_Variables.insert({var->name, var}).second;
    if (!inserted) {
        Console::Printf("Can't register variable %s, already defined.", var->name);
        return false;
    }
    return true;
}

bool VariableManager::RegisterAlias(ConsoleVariable* var, std::string alias) {
    if (CommandManager::Find(alias)) {
        Console::Printf("Can't register variable with alias, '%s' is a command.", alias);
        return false;
    } else if (Find(var->name) != var) {
        Console::Printf("Can't register variable with alias '%s', variable '%s' not registered properly.", alias, var->name);
        return false;
    }
    bool inserted = m_Variables.insert({alias, var}).second;
    if (!inserted) {
        Console::Printf("Can't register variable with alias %s, already defined.", alias);
        return false;
    }
    return true;
}

ConsoleVariable* VariableManager::Create(std::string name, float value) {
    ConsoleVariable* var = Find(name);
    if (var) {
        return var;
    } else if (CommandManager::Exists(name)) {
        Console::Printf("Can't create variable, '%s' is a command.", name);
        return nullptr;
    }
    var = new ConsoleVariable(name, value);
    Register(var);
    return var;
}

ConsoleVariable* VariableManager::Find(std::string name) {
    auto it = m_Variables.find(name);
    return it != m_Variables.end() ? it->second : nullptr;
}

bool VariableManager::Exists(std::string name) {
    return Find(name) != nullptr;
}

void VariableManager::Set(std::string name, float value) {
    ConsoleVariable* var = Find(name);
    if (!var) {
        Console::Printf("Variable '%s' not found.", name);
        return;
    }
    Set(var, value);
}

void VariableManager::Set(ConsoleVariable* var, float value) {
    // this setter is pretty pointless at the moment, but could become more complex in the future...
    var->value = value;
}

void VariableManager::Reset(std::string name) {
    ConsoleVariable* var = Find(name);
    if (!var) {
        Console::Printf("Variable '%s' not found.", name);
        return;
    }
    Set(var, var->defaultValue);
}

bool VariableManager::HandleCommand(std::vector<std::string> args) {
    ConsoleVariable* var = Find(args[0]);
    if (!var) return false; // TODO: also match substrings and show all matching variables?
        
    if (args.size() == 1) { // print variable info
        std::string fmt = "Variable '%s' is %.*f";
        int decimals = var->value == 0 || var->value == 1 ? 0 : 2;
        if (var->value == var->defaultValue) {
            Console::Printf(fmt + " (default)", args[0], decimals, var->value);
        } else {
            int defaultDecimals = var->defaultValue == 0 || var->defaultValue == 1 ? 0 : 2;
            Console::Printf(fmt + " (default: %.*f)", args[0], decimals, var->value, defaultDecimals, var->defaultValue);
        }
    } else if (args.size() == 2) { // set variable
        std::string argString = args[1];
        if ( !IsStringFloat(argString) ) {
            Console::Printf("Can't set variable, invalid value '%s'.", argString);
        }
        else {
            float newVal = atof( argString.c_str() );
            Set(var, newVal);
        }
    } else {
        Console::Printf("Invalind number of arguments to set variable.");
    }
    return true;
}
