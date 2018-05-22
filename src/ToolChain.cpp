#include "ToolChain.h"

#include <iostream>
#include <exception>

namespace tester {

ToolChain::ToolChain(const JSON &json) {
#ifdef _WIN32
  throw std::runtime_error("Don't know how to do validation on Windows yet.");
#endif

  for (const JSON &step : json["steps"])
    commands.emplace_back(step);
}

ExecutionOutput ToolChain::build(std::string inputFile) {
  ExecutionInput ei(inputFile);
  ExecutionOutput eo("");
  for (const Command &c : commands) {
    eo = c.execute(ei);
    ei = ExecutionInput(eo.getOutputFile());
  }

  return eo;
}

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
