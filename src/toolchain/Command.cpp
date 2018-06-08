#include "toolchain/Command.h"

#include "util.h"

#include "toolchain/CommandException.h"

#include <cstdlib>

namespace tester {

Command::Command(const JSON &step) {
  // Make sure the step has all of the values needed for construction.
  ensureContains(step, "stepName");
  ensureContains(step, "executablePath");
  ensureContains(step, "arguments");
  ensureContains(step, "output");

  // Build the command.
  name = step["stepName"];
  for (std::string arg : step["arguments"])
    args.push_back(arg);

  // Build the output.
  std::string outName = step["output"];

  // "-" represents stdout.
  if (outName == "-") {
#ifdef _WIN32
    throw std::runtime_error("Don't know how to capture stdout on Windows yet.");
#endif
    isStdOut = true;

    // Build a path in this directory for the standard output.
    fs::path fileName(name + "-temp.out");
    output = fs::current_path();
    output /= fileName;
  }
  // We've got a file name.
  else {
    isStdOut = false;

    // Make sure the output path is absolute.
    fs::path outPath(outName);
    if (outPath.is_absolute())
      output = outPath;
    else
      output = fs::absolute(outPath);
  }

  // Need to explicitly tell json what type we're pulling out here because it doesn't like loading
  // into an fs::path.
  std::string path = step["executablePath"];
  exePath = fs::path(path);
}

ExecutionOutput Command::execute(const ExecutionInput &ei) const {
  // Create our output context.
  ExecutionOutput eo(output);

  // Make the actual shell command, using our input and output contexts.
  std::string command = buildCommand(ei, eo);

  // Run the command. If we fail, raise a custom exception.
  int rv = std::system(command.c_str());

#if __linux__ || __unix__ || __unix
  // If we're on a POSIX system then we need to decompose the return value.
  rv = WEXITSTATUS(rv);
#endif

  if (rv != 0)
    throw CommandException("Subcommand returned status code " + std::to_string(rv) +
                           ":\n  " + command);

  // Tell the toolchain about our output.
  return eo;
}

std::string Command::buildCommand(const ExecutionInput &ei, const ExecutionOutput &eo) const {
  // We start with the path to the exe.
  std::string command = resolveExe(ei, eo, exePath).string();

  // Then add new arguments, using the resolver to see if they're "magic" arguments.
  for (std::string arg : args) {
    command += ' ';
    command += resolveArg(ei, eo, arg).string();
  }

  // If we were initially writing to stdout, then we add the redirect.
  if (isStdOut) {
#ifdef _WIN32
    throw std::runtime_error("Don't know how to capture stdout on Windows yet");
#endif
    command += " > \"" + eo.getOutputFile().string() + "\"";
  }

  return command;
}

fs::path Command::resolveArg(const ExecutionInput &ei,const ExecutionOutput &eo,
    std::string arg) const {
  // Input magic argument. Resolves to the input file for this command.
  if (arg == "$INPUT")
    return ei.getInputFile();

  // Output magic argument. Resolves to the output file for this command.
  if (arg == "$OUTPUT")
    return eo.getOutputFile();

  // Seem like it was meant to be a magic parameter.
  if (arg[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + arg);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(arg);
}

fs::path Command::resolveExe(const ExecutionInput &ei,const ExecutionOutput &eo,
                                std::string exe) const {
  // Exe magic argument. Resolves to the current "tested executable" (probably your compiler).
  if (exe == "$EXE")
    return ei.getTestedExecutable();

  // Input magic argument. Resolves to the input file for this command. Use for when a step
  // prodouces a runnable executable (your compiled executable).
  if (exe == "$INPUT")
    return ei.getInputFile();

  // Seem like it was meant to be a magic parameter.
  if (exe[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + exe);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(exe);
}

// Implement the Command ostream operator
std::ostream &operator<<(std::ostream &os, const Command &c) {
  ExecutionInput ei("$INPUT", "$EXE");
  ExecutionOutput eo(c.output);
  os << c.buildCommand(ei, eo);
  return os;
}

} // End namespace tester

