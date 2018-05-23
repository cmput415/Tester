#ifndef TESTER_TOOL_CHAIN_H
#define TESTER_TOOL_CHAIN_H

#include "toolchain/Command.h"

#include "json.hpp"

#include <string>
#include <vector>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// A simple toolchain that assumes that the output file of one step is the input file of the next
class ToolChain {
public:
  // There is no default constructor.
  ToolChain() = delete;

  // Construct the ToolChain from a json file path.
  explicit ToolChain(const JSON &json);

  // Runs the toolchain on a specified inputfile.
  ExecutionOutput build(std::string inputFile);

  // Ostream operator.
  friend std::ostream &operator<<(std::ostream&, const ToolChain&);

private:
  std::vector<Command> commands;
};

} // End namespace tester

#endif // TESTER_TOOL_CHAIN_H
