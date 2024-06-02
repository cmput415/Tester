#ifndef TESTER_TOOL_CHAIN_H
#define TESTER_TOOL_CHAIN_H

#include "toolchain/Command.h"
#include "tests/TestFile.h"
#include "json.hpp"

#include <string>
#include <vector>
#include <filesystem>

// Convenience.
using JSON = nlohmann::json;
namespace fs = std::filesystem;

namespace tester {

// A simple toolchain that assumes that the output file of one step is the input file of the next
class ToolChain {
public:
  // There is no default constructor.
  ToolChain() = delete;

  // Construct the ToolChain from a json file path.
  ToolChain(const JSON &json, int64_t timeout);

  // Copy constructor is default copy.
  ToolChain(const ToolChain &tc) = default;

  // Runs the toolchain on a specified inputfile.
  ExecutionOutput build(const std::unique_ptr<TestFile> &test) const;

  // Manipulate the executable to be tested.
  void setTestedExecutable(fs::path testedExecutable_) {
    testedExecutable = std::move(testedExecutable_);
  }

  // Manipulate the tested runtime.
  void setTestedRuntime(fs::path testedRuntime_) {
    testedRuntime = std::move(testedRuntime_);
  }

  // Gets a brief description of the toolchain.
  std::string getBriefDescription() const;

  // Ostream operator.
  friend std::ostream &operator<<(std::ostream&, const ToolChain&);

private:
  // The list of commands to execute this toolchain.
  std::vector<Command> commands;

  // The tested executable.
  fs::path testedExecutable;

  // The tested executable's runtime.
  fs::path testedRuntime;
};

} // End namespace tester

#endif // TESTER_TOOL_CHAIN_H
