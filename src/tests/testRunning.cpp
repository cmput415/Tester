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

#if defined(DEBUG)

  // std::cout << "Supplied Input Stream:" << std::endl; 
  // std::string line;
  // std::ifstream ins(test->getInsPath());
  
  // while (ins.good()) {
  //   std::getline(ins, line);
  //   std::cout << line << std::endl;
  // } 

  // std::cout << "Expected Output:" << std::endl; 
  // for (auto& line: test->getCheckLines()) {
  //   std::cout << line << std::endl;
  // }

#endif
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

  // TODO: test error tests
  bool isErrorTest = false;
  const std::vector<std::string> &checkLines = test->getCheckLines();
  if (checkLines.size() == 1 && checkLines[0].find("Error") != std::string::npos) {
    isErrorTest = true;
  }

  // Get the lines from the output file.
  std::vector<std::string> genLines;

  if (!isErrorTest) { // Is not an error test.
    getFileLines(eo.getOutputFile(), genLines);
  }
  else { // Is an error test.
    getFileLines(eo.getErrorFile(), genLines);
    if (!genLines.empty())
      genLines = {genLines[0].substr(0, genLines[0].find(':'))};
  }

  dtl::Diff<std::string> diff(checkLines, genLines);
  diff.compose();
  diff.composeUnifiedHunks(); 
    
  

  // We failed the test.
  if (!diff.getUniHunks().empty()) {
    // std::stringstream ss;
    // diff.printUnifiedFormat(ss);

    // DEBUG STUFF    
    std::cout << "Expected lines:" << "(" << test->getCheckLines().size() << ")" << std::endl;
    for (auto& line: test->getCheckLines()) {
      std::cout << line << std::endl;
    }
    std::cout << "Recieved:" << std::endl;
    for (auto& line: genLines) {
      std::cout << line << std::endl;
    }

    return TestResult(test->getTestPath(), false, false, "");
  }

  return TestResult(test->getTestPath(), true, false, "");
}

} // End namespace tester
