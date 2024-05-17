#include "toolchain/ToolChain.h"

#include "util.h"

#include <iostream>
#include <exception>

namespace tester {

ToolChain::ToolChain(const JSON &json, int64_t timeout) {
  // Make sure we've got an array of commands.
  if (!json.is_array())
    throw std::runtime_error("Not a toolchain array.");

  // Build our commands from each step.
  for (const JSON &step : json)
    commands.emplace_back(step, timeout);
}

ExecutionOutput ToolChain::build(const std::unique_ptr<TestFile>& test) const {

  ExecutionInput ei(test->testPath, test->inputStrPath, testedExecutable, testedRuntime);
  ExecutionOutput eo("");

  for (const Command &c : commands) {
    
    eo = c.execute(ei);

    std::error_code ec;
    std::uintmax_t olen = std::filesystem::file_size(eo.getOutputFile(), ec);
    if (olen == static_cast<std::uintmax_t>(-1) || olen == 0) {
        if (std::filesystem::exists(eo.getErrorFile(), ec)) {
            // Break the command pipe and return the current output
            return eo;
        }
    }

    ei = ExecutionInput(
      eo.getOutputFile(),
      ei.getInputStreamFile(),
      ei.getTestedExecutable(),
      ei.getTestedRuntime()
    );
  }

  return eo;
}

std::string ToolChain::getBriefDescription() const {
  std::string names = "";

  for (const Command &c : commands) {
    if (names != "")
      names += ", ";
    names += c.getName();
  }

  return "[" + names + "]";
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
