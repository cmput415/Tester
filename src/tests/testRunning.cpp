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
    while (fs.good()) {
        std::getline(fs, buf);
        lines.push_back(buf);
    }
}

} // End anonymous namespace


namespace tester {

TestResult runTest(const TestFile &test, const ToolChain &toolChain, bool quiet) {
  // Try to build the test. If there's a problem running a command, then we assume failure.
  ExecutionOutput eo("");
  std::string diff = "";

  
  try {
    eo = toolChain.build(test);
  }
  catch (const CommandException &ce) {
      if (!quiet) {
          std::cout << "Command error: " << ce.what() << '\n';
          std::cout << "output " << eo.getOutputFile() << std::endl;
      }
      return TestResult(test, false, true, "");
  }

  // std::cout << "Output File: !" << eo.getOutputFile() << std::endl;
  std::vector<std::string> genLines; 
  getFileLines(eo.getOutputFile(), genLines);

  std::cout << "CHECK LINES" << std::endl;
  for (auto line : test.checkLines) {
    std::cout << "\tCHECK: " << line << std::endl;
  }

  std::cout << "GEN LINES" << std::endl;

  for (auto line : genLines) {
    std::cout << "\tGEN: " << line << std::endl;
  }  
  // Read 

  return TestResult(test, true, false, diff);  
  

  // // Get the lines from the reference file.


  // /* Check to see if this an error test. The expected output must contain
  //  * exactly one line and the substring "Error".
  //  */
  // bool isErrorTest = false;
  // if (expLines.size() == 1 && expLines[0].find("Error") != std::string::npos) {
  //   isErrorTest = true;
  //   expLines = {expLines[0]};
  // }

  // // Get the lines from the output file.
  // std::vector<std::string> genLines;

  // if (!isErrorTest) { // Is not an error test.
  //   getFileLines(eo.getOutputFile(), genLines);
  // }
  // else { // Is an error test.
  //   getFileLines(eo.getErrorFile(), genLines);
  //   if (!genLines.empty())
  //     genLines = {genLines[0].substr(0, genLines[0].find(':'))};
  // }

  // dtl::Diff<std::string> diff(expLines, genLines);
  // diff.compose();
  // diff.composeUnifiedHunks();

  // // We failed the test.
  // if (!diff.getUniHunks().empty()) {
  //   std::stringstream ss;
  //   diff.printUnifiedFormat(ss);
  //   return TestResult(pm.in, false, false, ss.str());
  // }

  // return TestResult(pm.in, true, false, "");
}

} // End namespace tester
