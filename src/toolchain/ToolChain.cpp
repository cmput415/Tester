#include "toolchain/ToolChain.h"

#include "util.h"

#include <exception>
#include <iostream>

namespace tester {

ToolChain::ToolChain(const JSON& json, int64_t timeout) {
  // Make sure we've got an array of commands.
  if (!json.is_array())
    throw std::runtime_error("Not a toolchain array.");

  // Build our commands from each step.
  for (const JSON& step : json)
    commands.emplace_back(step, timeout);
}

ExecutionOutput ToolChain::build(TestFile* test) const {
  // The current output and input contexts.
  ExecutionInput ei(test->getTestPath(), test->getInsPath(), testedExecutable, testedRuntime);
  ExecutionOutput eo;

  // Run the command, updating the contexts as we go.
  auto last = commands.end() - 1;
  for (auto it = commands.begin(); it != commands.end(); it++) {
    
    eo = it->execute(ei);
    int rv = eo.getReturnValue();
    
    // Terminate the toolchain prematurely if we encounter a non-zero exit status
    // or if the error stream has bytes. 
    if (rv != 0 || fs::file_size(eo.getErrorFile()) != 0) {
      eo.setIsErrorTest(true);
      return eo;
    }
     
    ei = ExecutionInput(eo.getOutputFile(), ei.getInputStreamFile(), ei.getTestedExecutable(),
                        ei.getTestedRuntime());
  }

  // store the elapsed time of the final step execution step into the testfile
  std::optional<double> elapsedTime = eo.getElapsedTime();
  if (elapsedTime.has_value()) {
    test->setElapsedTime(elapsedTime.value());
  } 

  return eo;
}

std::string ToolChain::getBriefDescription() const {
  std::string names = "";

  for (const Command& c : commands) {
    if (names != "")
      names += ", ";
    names += c.getName();
  }

  return "[" + names + "]";
}

// Implement ostream operator for the toolchain.
std::ostream& operator<<(std::ostream& os, const ToolChain& tc) {
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