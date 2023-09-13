#include "tests/testUtil.h"

#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"

#include "dtl/dtl.hpp"

#include <fstream>
#include <sstream>

// Private namespace that holds utility functions for the functions that are actually exported. You
// can find the actual functions at the bottom of the file.
namespace {

void getFileLines(fs::path fp, std::vector<std::string> &lines) {
    std::ifstream fs(fp);
    std::string buf;
    while (!fs.eof()) {
        std::getline(fs, buf);
        lines.push_back(buf);
    }
}

} // End anonymous namespace


namespace tester {

TestResult runTest(const PathMatch &pm, const ToolChain &toolChain, bool quiet) {
  // Try to build the test. If there's a problem running a command, then we assume failure.
  ExecutionOutput eo("");
  try {
      eo = toolChain.build(pm.in, pm.inStream);
  }
  catch (const CommandException &ce) {
      if (!quiet) {
          std::cout << "Command error: " << ce.what() << '\n';
          std::cout << "output " << eo.getOutputFile() << std::endl;
      }
      return TestResult(pm.in, false, true, "");
  }

  // Get the lines from the reference file.
  std::vector<std::string> expLines;
  getFileLines(pm.out, expLines);

  /* Check to see if this an error test. The expected output must contain
   * exactly one line and the substring "Error".
   */
  bool isErrorTest = false;
  if (expLines.size() == 1 && expLines[0].find("Error") != std::string::npos) {
    isErrorTest = true;
    expLines = {expLines[0]};
  }

  // Get the lines from the output file.
  std::vector<std::string> genLines;

  if (!isErrorTest) { // Is not an error test.
    getFileLines(eo.getOutputFile(), genLines);
  }
  else { // Is an error test.
    getFileLines(eo.getErrorFile(), genLines);
    genLines = {genLines[0].substr(0, genLines[0].find(':'))};
  }

  dtl::Diff<std::string> diff(expLines, genLines);
  diff.compose();
  diff.composeUnifiedHunks();

  // We failed the test.
  if (!diff.getUniHunks().empty()) {
    std::stringstream ss;
    diff.printUnifiedFormat(ss);
    return TestResult(pm.in, false, false, ss.str());
  }

  return TestResult(pm.in, true, false, "");
}

} // End namespace tester
