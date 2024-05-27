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
namespace {

void dumpFile(const fs::path& filePath) {
  
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filePath << std::endl;
    return;
  }
  std::cout << ">>>" << std::endl;
  char ch;
  while (file.get(ch)) {
    if (ch == ' ') {
      std::cout << '*';
    } else {
      std::cout << ch;
    }
  }

  file.close();
  std::cout << "<<<" << std::endl;
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


void getErrorSubstr(std::string &line) {

  std::string errorUpper = "Error:", errorLower = "error:";

  size_t pos1 = line.find(errorUpper);
  size_t pos2 = line.find(errorLower);

  if (pos1 != std::string::npos) {
    line = line.substr(0, pos1 + errorUpper.length());
  } else if (pos2 != std::string::npos) {
    line = line.substr(0, pos2 + errorLower.length());
  } else {
    line = "";
  }
}

/**
 * @brief Tests that produce errors may contian some non-deterministic messages like PID that
 * we can not make checks for in advance. This method does a weaker partial match on the LHS of
 * the first occurence of the keyword Error: or error for bothl files.
 * 
 * @returns False if there is no difference, True otherwise. Wrapped in a std::pair with diff
 * string.
 *  
 * @example "MathError: line 8", "MathError: line 10" FALSE
 * @example "error: 10291 (segmentation fault)", "error: 10295 (segmentation fault)" FALSE
 * @example "TypError:", "MathError:" TRUE
*/
std::pair<bool, std::string> diffErrorFiles(const fs::path& file1, const fs::path& file2) {
  
  std::string diffStr = "";  
  std::ifstream ifs1(file1), ifs2(file2);

  if (!ifs1.is_open() || !ifs2.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }

  std::string errorLineOne, errorLineTwo; 
  std::getline(ifs1, errorLineOne);
  std::getline(ifs2, errorLineTwo);

  getErrorSubstr(errorLineOne);
  getErrorSubstr(errorLineTwo);
  
  if (errorLineOne == errorLineTwo) {
    return std::make_pair(false, "");
  } else {
    std::string diff = "+ " + errorLineOne + "\n- " + errorLineTwo;
    return std::make_pair(true, std::move(diff));
  }
}

} // End anonymous namespace


namespace tester {

TestResult runTest(const std::unique_ptr<TestFile> &test, const ToolChain &toolChain, const Config &cfg) {

  // Try to build the test. If there's a problem running a command, then we assume failure.
  ExecutionOutput eo("");
  try {
    eo = toolChain.build(test->getTestPath(), test->getInsPath());
    if (eo.getReturnValue() != 0) {
      
      // If the toolchain allows errors then a test with non-zero exist status may propogate here.
      // If the stderr matches (partially with keywords), the expected check value,
      // then the test will still pass. 
      if (test->usesOut) {
        std::pair<bool, std::string> diff = diffErrorFiles(eo.getErrorFile(), test->getOutPath());
        if (diff.first) {
          // test produces and error and stderr does not match the expected output
          return TestResult(test->getTestPath(), false, true, diff.second);
        } else {
          // test produced an error it expected
          return TestResult(test->getTestPath(), true, true, "");
        }
      } else {
        return TestResult(test->getTestPath(), false, true, "Test does not supply expected output.");
      }      
    }
  } catch (const CommandException &ce) {
    if (!cfg.isQuiet()) {
      std::cout << "Command error: " << ce.what() << '\n';
      std::cout << "output " << eo.getOutputFile() << std::endl;
    }
    return TestResult(test->getTestPath(), false, true, "");
  }

  std::pair<bool, std::string> diff = diffFiles(eo.getOutputFile(), test->getOutPath());
  if (diff.first) {
    return TestResult(test->getTestPath(), false, false, diff.second); 
  }

  return TestResult(test->getTestPath(), true, false, "");
}

} // End namespace tester
