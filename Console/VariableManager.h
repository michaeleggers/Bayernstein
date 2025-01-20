#ifndef VARIABLEMANAGER_H
#define VARIABLEMANAGER_H

#include <map>
#include <string>
#include <vector>

struct ConsoleVariable {
    const std::string name;
    const float       defaultValue;
    float             value;
    // TODO: maybe store entered value string (like quake) to avoid formatting issues for printing (esp. with floats)? see `HandleCommand()`.
    // TODO: optionally store min/max value (and maybe stepsize?) for input validation?
    // TODO: add description / help text for console help?
    ConsoleVariable(std::string n, float v)
        : name(n),
          defaultValue(v),
          value(v) {}
};

/**
 * Manages variables that are accessible from the console.
 * Loosely based on Quake's (QSS-M) `cvar` implementation.
 */
class VariableManager {
  private:
    /** List of registered variables, mapped by their (unique) name. */
    static std::map<std::string, ConsoleVariable*> m_Variables;

  public:
    /** Initialize VariableManager variables / commands. */
    static void Init();
    /** Registers the given variable to be available in the console. */
    static bool Register(ConsoleVariable* var);
    /** Registers an existing (and registered) variable with the given alias name. */
    static bool RegisterAlias(ConsoleVariable* var, std::string alias);
    /** Creates a new variable and registers it. */
    static ConsoleVariable* Create(std::string name, float value);
    /** Finds the variable with the given name in the registered variables. */
    static ConsoleVariable* Find(std::string name);
    /** Checks if a variable with the given name is registered. */
    static bool Exists(std::string name);
    /** Sets the value of the variable with the given name. */
    static void Set(std::string name, float value);
    /** Sets the value of the given variable reference. */
    static void Set(ConsoleVariable* var, float value);
    /** Resets the variable with the given name to its default value. */
    static void Reset(std::string name);
    /** Gets a list of all registered variables. */
    static std::vector<ConsoleVariable*> GetAll();

    /**
     * Handles the user command input (similar to a `CommandHandler` function for console commands), expecting a variable name as command.
     * @returns `true` if handled (variable exists), `false` otherwise.
     */
    static bool HandleCommand(std::vector<std::string> args);

    VariableManager() = delete; // don't allow instantiation
};

#endif // VARIABLEMANAGER_H
