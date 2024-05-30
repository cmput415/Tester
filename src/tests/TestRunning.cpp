#include "tests/Util.h"
#include "tests/TestFile.h"

#include "config/Config.h"
#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"
#include "dtl/dtl.hpp"

#include <tuple>
#include <fstream>
#include <sstream>

namespace {

void dumpFile(const fs::path& filePath, bool showSpace=false) {
  
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filePath << std::endl;
    return;
  }
  std::cout << "--->" << std::endl;
  char ch;
  while (file.get(ch)) {
    if (ch == ' ' && showSpace) {
      std::cout << '*';
    } else {
      std::cout << ch;
    }
  }

  file.close();
  std::cout << "<---" << std::endl;
}


std::pair<bool, std::string> diffFiles(const fs::path& file1, const fs::path& file2) {
  std::vector<std::string> lines1, lines2;
  std::ifstream ifs1(file1);
  std::ifstream ifs2(file2);

  if (!ifs1.is_open() || !ifs2.is_open()) {
    std::cerr << "Error opening files." << std::endl;
    return std::make_pair(true, "");
  }

  std::string line;
  while (std::getline(ifs1, line)) {
    lines1.push_back(line);
  }

  while (std::getline(ifs2, line)) {
    lines2.push_back(line);
  }

  dtl::Diff<std::string> diff(lines1, lines2);
  diff.compose();
  dtl::Ses<std::string> ses = diff.getSes();

  std::string diffStr;
  bool isDifferent = false;

  for (auto& sesElem : ses.getSequence()) {
    if (sesElem.second.type != dtl::SES_COMMON) {
      diffStr += sesElem.second.type == dtl::SES_ADD ? "+ " : "- ";
      isDifferent = true;
    }
  }

  return std::make_pair(isDifferent, std::move(diffStr));
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
    eo = toolChain.build(test);
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
      dumpFile(test->getOutPath(), true);
      dumpFile(eo.getErrorFile(), true); 
      return TestResult(test->getTestPath(), false, false, ""); 
    }

    return TestResult(test->getTestPath(), true, false, "");
  }

}

} // End namespace tester
