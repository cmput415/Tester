#include "toolchain/ToolChain.h"

#include "util.h"

#include <iostream>
#include <exception>

namespace tester {

ToolChain::ToolChain(const JSON &json) {
#ifdef _WIN32
  throw std::runtime_error("Don't know how to do validation on Windows yet.");
#endif

  // Make sure steps exists before creating the toolchain.
  ensureContains(json, "steps");
  for (const JSON &step : json["steps"])
    commands.emplace_back(step);
}

ExecutionOutput ToolChain::build(fs::path inputPath) const {
  // The current output and input contexts.
  ExecutionInput ei(inputPath);
  ExecutionOutput eo("");

  // Run the command, updating the contexts as we go.
  for (const Command &c : commands) {
    eo = c.execute(ei);
    ei = ExecutionInput(eo.getOutputFile());
  }

  // Return the output context of the final command.
  return eo;
}

// Implement ostream operator for the toolchain.
std::ostream &operator<<(std::ostream &os, const ToolChain &tc) {
  os << "Toolchain: \n";
  size_t size = tc.commands.size();
  for (size_t i = 0; i < size; ++i) {
    os << "  Command " << i << ": " << tc.commands[i];
    if (i < size - 1)
      os << '\n';
  }

  return os;
}

} // End namespace tester
