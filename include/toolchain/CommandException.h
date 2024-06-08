#ifndef TESTER_COMMAND_EXCEPTION_H
#define TESTER_COMMAND EXCEPTION_H

#include <exception>

namespace tester {

// Represents a problem running a command.
class CommandException : public std::runtime_error {
public:
  explicit CommandException(std::string s) : std::runtime_error(s) {}
};

class FailException : public CommandException {
public:
  explicit FailException(std::string s) : CommandException(s) {}
};

class TimeoutException : public CommandException {
public:
  explicit TimeoutException(std::string s) : CommandException(s) {}
};

} // End namespace tester

#endif // TESTER_COMMAND EXCEPTION_H
