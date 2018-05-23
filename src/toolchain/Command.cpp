#include "toolchain/Command.h"

#include <exception>
#include <cstdlib>

namespace tester {

Command::Command(const JSON &step) {
  name = step["stepName"];
  exePath = step["executablePath"];
  for (std::string arg : step["arguments"])
    args.push_back(arg);
  output = step["output"];
}

ExecutionOutput Command::execute(const ExecutionInput &ei) const {
  std::string outputPath;

  if (output == "-") {
#ifdef _WIN32
    throw std::runtime_error("Don't know how to capture stdout on Windows yet");
#endif
    outputPath = generateOutputName(ei);
  }
  else
    outputPath = output;

  ExecutionOutput eo(outputPath);

  std::string command = buildCommand(ei, eo);

  int rv = std::system(command.c_str());
  if (rv != 0)
    throw std::runtime_error("Subcommand returned status code " + std::to_string(rv) +
                             ": " + command);

  return eo;

}

std::string Command::buildCommand(const ExecutionInput &ei, const ExecutionOutput &eo) const {
  std::string command = exePath;

  for (std::string arg : args) {
    command += ' ';
    command += resolveArg(ei, eo, arg);
  }

  if (output == "-") {
#ifdef _WIN32
    throw std::runtime_error("Don't know how to capture stdout on Windows yet");
#endif
    command += " > " + eo.getOutputFile().string();
  }

  return command;
}

std::string Command::resolveArg(const ExecutionInput &ei,const ExecutionOutput &eo,
    std::string arg) const {
  if (arg == "$INPUT")
    return ei.getInputFile();

  if (arg == "$OUTPUT")
    return eo.getOutputFile();

  // Seem like it was meant to be a magic parameter
  if (arg[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + arg);

  // Wasn't a special arg, we should just return the arg
  return arg;
}

std::string Command::generateOutputName(const ExecutionInput &ei) const {
  return name + "-temp.out";
}

// Implement the Command ostream operator
std::ostream &operator<<(std::ostream &os, const Command &c) {
  ExecutionInput ei("$INPUT");
  ExecutionOutput eo(c.generateOutputName(ei));
  os << c.buildCommand(ei, eo);
  return os;
}

} // End namespace tester

