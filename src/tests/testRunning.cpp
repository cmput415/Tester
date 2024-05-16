#include "tests/testUtil.h"

#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"

#include "dtl/dtl.hpp"

#include <fstream>
#include <sstream>

void getFileLines(fs::path fp, std::vector<std::string> &lines) {
  std::ifstream fs(fp);
  std::string buf;
  while (fs.good()) {
    std::getline(fs, buf);
    lines.push_back(buf);
  }
}

namespace tester {

TestResult runTest(const std::unique_ptr<TestFile> &test, const ToolChain &toolChain, bool quiet) {
  // Try to build the test. If there's a problem running a command, then we assume failure.
  ExecutionOutput eo("");
 
  try { 
    eo = toolChain.build(test);
  }
  catch (const CommandException &ce) {
    if (!quiet) {
      std::cout << "Command error: " << ce.what() << '\n';
      std::cout << "output " << eo.getOutputFile() << std::endl;
    }
    return TestResult(test->getTestPath(), false, true, "");
  }

  std::vector<std::string> genLines; 
  getFileLines(eo.getOutputFile(), genLines);

  dtl::Diff<std::string> diff(test->getCheckLines(), genLines);
  diff.compose();
  diff.composeUnifiedHunks();

  // std::cout << "DEBUG" << std::endl;
  // for (auto line: test->checkLines) {
  //   std::cout << "\tCHECK:" << line << std::endl;
  // }
  // for (auto line: genLines) {
  //   std::cout << "\tGEN:" << line << std::endl;
  // }

  // We failed the test.
  if (!diff.getUniHunks().empty()) {
    std::stringstream ss;
    ss << "[EXPECTED]\n";
    ss << "[RECEIVED]\n";
    // std::cout << "[EXPECTED]" << std::endl;
    
    // std::cout << "[RECEIVED]" << std::endl;

    return TestResult(test->getTestPath(), false, false, ss.str());
  }

  return TestResult(test->getTestPath(), true, false, "");
}

} // End namespace tester
