#ifndef TESTER_TOOL_CHAIN_H
#define TESTER_TOOL_CHAIN_H

#include "toolchain/Command.h"

#include "json.hpp"

#include <string>
#include <vector>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;
namespace fs = std::experimental::filesystem;

namespace tester {

// A simple toolchain that assumes that the output file of one step is the input file of the next
class ToolChain {
public:
  // There is no default constructor.
  ToolChain() = delete;

  // Construct the ToolChain from a json file path.
  explicit ToolChain(const JSON &json);

  // Runs the toolchain on a specified inputfile.
  ExecutionOutput build(fs::path inputPath) const;

  // Set the executable to be tested.
  void setTestedExecutable(fs::path testedExecutable_) { testedExecutable = testedExecutable_; }

  // Ostream operator.
  friend std::ostream &operator<<(std::ostream&, const ToolChain&);

private:
  // The list of commands to execute this toolchain.
  std::vector<Command> commands;

  fs::path testedExecutable;
};

} // End namespace tester

#endif // TESTER_TOOL_CHAIN_H
