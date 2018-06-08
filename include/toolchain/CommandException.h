#ifndef TESTER_COMMAND_EXCEPTION_H
#define TESTER_COMMAND EXCEPTION_H

#include <exception>

namespace tester {

// Represents a problem running a command.
class CommandException : public std::runtime_error {
public:
  explicit CommandException(std::string s) : std::runtime_error(s) { }
};

} // End namespace tester

#endif //TESTER_COMMAND EXCEPTION_H
