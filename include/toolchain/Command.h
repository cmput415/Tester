#ifndef TESTER_COMMAND_H
#define TESTER_COMMAND_H

#include "json.hpp"

#include "toolchain/ExecutionState.h"

#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// A class representing a command in a toolchain.
class Command {
public:
  // No default constructor.
  Command() = delete;

  // Construct a command from JSON set up.
  explicit Command(const JSON &json);

  // Execute the command.
  ExecutionOutput execute(const ExecutionInput &ei) const;

  // Get the command name.
  std::string getName() const { return name; }

  // Ostream operator.
  friend std::ostream &operator<<(std::ostream&, const Command&);

private:
  // Builds the actual CLI command represented by this object.
  std::string buildCommand(const ExecutionInput &input, const ExecutionOutput &output) const;

  // Resolves magic parameters to values.
  std::string resolveArg(const ExecutionInput &ei, const ExecutionOutput &eo, std::string arg)
    const;

  // Resolves magic exe parameters to value.
  std::string resolveExe(const ExecutionInput &ei, const ExecutionOutput &eo, std::string exe)
    const;

private:
  std::string name;
  fs::path exePath;
  std::vector<std::string> args;

  // Output info.
  bool isStdOut;
  fs::path output;
};

} // End namespace tester

#endif // TESTER_COMMAND_H
