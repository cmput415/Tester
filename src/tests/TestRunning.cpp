#include "tests/Util.h"
#include "tests/TestFile.h"

#include "config/Config.h"
#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"
#include "dtl/dtl.hpp"

#include <tuple>
#include <fstream>
#include <sstream>

// Private namespace that holds utility functions for the functions that are actually exported. You
// can find the actual functions at the bottom of the file.
namespace { } // End anonymous namespace

// void getErrorSubstr(std::string &line) {

//   std::string errorUpper = "Error:", errorLower = "error:";

//   size_t pos1 = line.find(errorUpper);
//   size_t pos2 = line.find(errorLower);

//   if (pos1 != std::string::npos) {
//     line = line.substr(0, pos1 + errorUpper.length());
//   } else if (pos2 != std::string::npos) {
//     line = line.substr(0, pos2 + errorLower.length());
//   } else {
//     line = "";
//   }
// }

// /**
//  * @brief Tests that produce errors may contian some non-deterministic messages like PID that
//  * we can not make checks for in advance. This method does a weaker partial match on the LHS of
//  * the first occurence of the keyword Error: or error for bothl files.
//  * 
//  * @returns False if there is no difference, True otherwise. Wrapped in a std::pair with diff
//  * string.
//  *  
//  * @example "MathError: line 8", "MathError: line 10" FALSE
//  * @example "error: 10291 (segmentation fault)", "error: 10295 (segmentation fault)" FALSE
//  * @example "TypError:", "MathError:" TRUE
// */
// std::pair<bool, std::string> diffErrorFiles(const fs::path& file1, const fs::path& file2) {
  
//   std::string diffStr = "";  
//   std::ifstream ifs1(file1), ifs2(file2);

//   if (!ifs1.is_open() || !ifs2.is_open()) {
//     throw std::runtime_error("Failed to open file.");
//   }

//   std::string errorLineOne, errorLineTwo; 
//   std::getline(ifs1, errorLineOne);
//   std::getline(ifs2, errorLineTwo);

//   getErrorSubstr(errorLineOne);
//   getErrorSubstr(errorLineTwo);
  
//   if (errorLineOne == errorLineTwo && (errorLineOne != "" && errorLineTwo != "")) {
//     return std::make_pair(false, "");
//   } else {
//     std::string diff = "+ " + errorLineOne + "\n- " + errorLineTwo;
//     return std::make_pair(true, std::move(diff));
//   }
// }


// bool testEmittedError(const fs::path& stdoutPath) {
//   std::ifstream ifs(stdoutPath);
//   if (!ifs.is_open()) {
//     throw std::runtime_error("Failed to open output file.");
//   }

//   // std::string firstLine
// }




/*


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
    if (!genLines.empty())
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

*/

namespace {

void dumpFile(const fs::path& filePath) {
  
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filePath << std::endl;
    return;
  }
  std::cout << "--->" << std::endl;
  char ch;
  while (file.get(ch)) {
    if (ch == ' ') {
      std::cout << '*';
    } else {
      std::cout << ch;
    }
  }

  file.close();
  std::cout << "<---" << std::endl;
}


std::pair<bool, std::string> diffFiles(const fs::path& file1, const fs::path& file2) {

  std::string diffStr = "";  
  std::ifstream ifs1(file1);
  std::ifstream ifs2(file2);

  if (!ifs1.is_open() || !ifs2.is_open()) {
    std::cerr << "Error opening files." << std::endl;
    return std::make_pair(false, "");
  }

  std::string line1, line2;
  bool diff = false;
  int lineNum = 1;

  while (std::getline(ifs1, line1) && std::getline(ifs2, line2)) {
    if (line1 != line2) {
      diffStr += "Line " + std::to_string(lineNum) + ":\n";
      diffStr += "- " + line1 + "\n";
      diffStr += "+ " + line2 + "\n";
      diff = true;
    }
    lineNum++;
  }

  // check for remaining lines in either file
  while (std::getline(ifs1, line1)) {
    diffStr += "Line " + std::to_string(lineNum) + ":\n";
    diffStr += "- " + line1 + "\n"; 
    diff = true;
    lineNum++; 
  }

  while (std::getline(ifs2, line2)) {
    diffStr += "Line " + std::to_string(lineNum) + ":\n";
    diffStr += "+ " + line1 + "\n"; 
    diff = true;
    lineNum++;
  }

  return std::make_pair(diff, std::move(diffStr));
}

/**
 * Given a file path, return the substring of the first line that conforms to the
 * error testcase specification. 
*/
std::string getErrorString(const fs::path stdOutPath) {

  std::ifstream ins(stdOutPath); // open input file stream of output file of toolchain
  if (!ins.is_open()) {
    throw std::runtime_error("Failed to open the generated output file of the toolchain.");
  }


  std::string firstLine;
  if (!getline(ins, firstLine)) {
    return std::string("");  
  }

  if (firstLine.find("Error") != std::string::npos) {

    // expected output matches the spec, return first line
    size_t colonPos = firstLine.find(":");
    if (colonPos == std::string::npos) {
      // this file only contains only the LHS of the real error output.
      return firstLine;
    }
    return firstLine.substr(0, firstLine.find(":"));
  }
  
  return std::string("");
}

}

namespace tester {

TestResult runTest(const std::unique_ptr<TestFile> &test, const ToolChain &toolChain, const Config &cfg) {

  ExecutionOutput eo("");

  const fs::path testPath = test->getTestPath();
  const fs::path insPath = test->getInsPath();

  try {
    eo = toolChain.build(testPath, test->getInsPath());
  } catch (const CommandException &ce) {
    // toolchain throws errors only when allowError is false in the config
    if (!cfg.isQuiet()) {
      std::cout << "Command error: " << ce.what() << '\n';
      std::cout << "output " << eo.getOutputFile() << std::endl;
    }
    return TestResult(testPath, false, true, "");
  }
  
  
  // Error test that propogated here since allowError is true in the config. 
  std::string genErrorString = getErrorString(eo.getErrorFile());
  std::string expErrorString = getErrorString(test->getOutPath());

  if (eo.getReturnValue() != 0 && !genErrorString.empty() && !expErrorString.empty()) {
 
    if (genErrorString == expErrorString) {
      return TestResult(testPath, true, true, "");
    }

    std::cout << "Exp and Gen strings don't match" << std::endl;
    return TestResult(testPath, false, true, "");

  } else {

    // Not an error test. Match the generated and expected outputs with exact diff.
    std::pair<bool, std::string> diff = diffFiles(eo.getOutputFile(), test->getOutPath());
    if (diff.first) {
      std::cout << "Difference between expected and generated output" << std::endl;
      dumpFile(test->getTestPath());
      dumpFile(test->getOutPath());
      dumpFile(eo.getErrorFile()); 
      return TestResult(test->getTestPath(), false, false, ""); 
    }

    return TestResult(test->getTestPath(), true, false, "");
  }

}

} // End namespace tester
