#include "tests/testUtil.h"

#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"

#include "dtl/dtl.hpp"

#include <fstream>

// Private namespace that holds utility functions for the functions that are actually exported. You
// can find the actual functions at the bottom of the file.
namespace {

void getFileLines(fs::path fp, std::vector<std::string> &lines) {
  std::ifstream fs(fp);
  std::string buf;
  while (std::getline(fs, buf))
    lines.push_back(buf);
}

} // End anonymous namespace


namespace tester {

TestResult runTest(const PathPair &tp, const ToolChain &toolChain, bool quiet) {
  // Try to build the test. If there's a problem running a command, then we assume failure.
  fs::path output;
  try {
    ExecutionOutput eo = toolChain.build(tp.in);
    output = eo.getOutputFile();
  }
  catch (const CommandException &ce) {
    if (quiet)
      std::cout << "Command error: " << ce.what() << '\n';
    return TestResult(tp.in, false, true, "");
  }

  // Get the lines from the files.
  std::vector<std::string> expLines;
  std::vector<std::string> genLines;
  getFileLines(tp.out, expLines);
  getFileLines(output, genLines);

  dtl::Diff<std::string> diff(expLines, genLines);
  diff.compose();
  diff.composeUnifiedHunks();

  // We failed the test.
  if (!diff.getUniHunks().empty()) {
    std::stringstream ss;
    diff.printUnifiedFormat(ss);
    return TestResult(tp.in, false, false, ss.str());
  }

  return TestResult(tp.in, true, false, "");
}

} // End namespace tester
