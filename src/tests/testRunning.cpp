#include "tests/Util.h"
#include "tests/TestFile.h"

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
    while (fs.good()) {
        std::getline(fs, buf);
        lines.push_back(buf);
    }
}

} // End anonymous namespace


namespace tester {

TestResult runTest(const std::unique_ptr<TestFile> &test, const ToolChain &toolChain, bool quiet) {

  // Try to build the test. If there's a problem running a command, then we assume failure.
  ExecutionOutput eo("");
  try {
      eo = toolChain.build(test->getTestPath(), test->getInsPath());
  }
  catch (const CommandException &ce) {
      if (!quiet) {
          std::cout << "Command error: " << ce.what() << '\n';
          std::cout << "output " << eo.getOutputFile() << std::endl;
      }
      return TestResult(test->getTestPath(), false, true, "");
  }

  // lines to check  
  const std::vector<std::string> &checkLines = test->getCheckLines();
  std::vector<std::string> genLines;

  // TODO: investigate why ExecutionOutput instances are created by passing stdoutPath as stderrPath 
  getFileLines(eo.getErrorFile(), genLines);


  std::cout << "Gen File:" << eo.getErrorFile() << std::endl;
  std::cout << "Out File:" << test->getOutPath() << std::endl; 

  dtl::Diff<std::string> diff(checkLines, genLines);
  diff.compose();
  diff.composeUnifiedHunks();  

  // We failed the test.
  if (!diff.getUniHunks().empty()) {

    // DEBUG
    std::cout << "Expected lines:" << "(" << test->getCheckLines().size() << ")" << std::endl;
    for (auto& line: test->getCheckLines()) {
      std::cout << line << std::endl;
    }
    std::cout << "Received lines:" << "(" << genLines.size() << ")" << std::endl;
    for (auto& line: genLines) {
      std::cout << line << std::endl;
    }

    return TestResult(test->getTestPath(), false, false, "");
  }

  return TestResult(test->getTestPath(), true, false, "");
}

} // End namespace tester
