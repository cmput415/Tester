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

  /* Check to see if this an error test. The error message doesn't have to be
   * first or only line of output (e.g. a runtime error). However, we are
   * currently restricting ourselves to the first error in the reference.
   */
  std::string refError;
  for (auto &line: expLines) {
      if (line.find("Error") != std::string::npos) {
          refError = line.substr(0, line.find(":")); 
          break;
      }
  }

  // Get the lines from the output file.
  std::vector<std::string> genLines;

  if (refError.empty()) { // Not an error test
      getFileLines(eo.getOutputFile(), genLines);

      dtl::Diff<std::string> diff(expLines, genLines);
      diff.compose();
      diff.composeUnifiedHunks();

      // We failed the test.
      if (!diff.getUniHunks().empty()) {
          std::stringstream ss;
          diff.printUnifiedFormat(ss);
          return TestResult(pm.in, false, false, ss.str());
      }
  }
  else { // error test
      getFileLines(eo.getErrorFile(), genLines);
      std::string genError;
      for (auto &line: genLines) {
          if (line.find("Error") != std::string::npos) {
              genError = line.substr(0, line.find(":")); 
              break;
          }
      }

      /* The size check is partially to avoid overflowing the filler, but also
       * in case the generated error message does not include the ':' token.
       */
      if (!genError.empty() && genError.size()<128) {
          int refErrLine, genErrLine;
          char filler[128];

          if (sscanf(refError.c_str(), "Error %s %d", filler, &refErrLine) != 2) {
              // Ill-formed reference string. Consider this an error.
              std::cerr << "Hmm, failed to parse ref error line?" << std::endl;
              return TestResult(pm.in, false, true, refError);
          }
          if (sscanf(genError.c_str(), "Error %s %d", filler, &genErrLine) != 2) {
              // Ill-formed generated error string. Consider this a fail.
              std::cerr << "Failed to parse generated error line" << std::endl;
              return TestResult(pm.in, false, false, genError);
          }
          if (refErrLine != genErrLine) {
              // Wrong line number. Is there room here for lee-way?
              std::string diff = refError + "\n-----------------\n" + genError;
              std::cerr << "Error lines differ: " << refErrLine << " != "
                        << genErrLine  << std::endl;
              return TestResult(pm.in, false, false, diff);
          }
      }
              
  }
      
  return TestResult(pm.in, true, false, "");
}

} // End namespace tester
