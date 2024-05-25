#include "tests/Util.h"
#include "tests/TestFile.h"

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

  std::pair<bool, std::string> diff = diffFiles(eo.getErrorFile(), test->getOutPath());
  if (diff.first) {
    dumpFile(test->getInsPath());
    dumpFile(test->getOutPath());
    dumpFile(eo.getErrorFile());
    return TestResult(test->getTestPath(), false, false, diff.second); 
  }
  
  return TestResult(test->getTestPath(), true, false, "");
}

} // End namespace tester
