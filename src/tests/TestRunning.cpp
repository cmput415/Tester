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

void dumpFile(const std::filesystem::path& filePath, bool showSpace=false) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Print some meta-data about the file.
    std::cout << "File Path: " << filePath << std::endl;
    std::cout << "File Size: " << fs::file_size(filePath) << std::endl;
    std::string delimiterStart = "====[ START FILE: " + filePath.stem().string() + " ]====";
    std::string delimiterEnd = "====[ END FILE: " + filePath.stem().string() + " ]====";
    std::cout << delimiterStart << std::endl;
    
    char ch;
    bool lastCharIsNl = false;
    while (file.get(ch)) {
        if (ch == ' ' && showSpace) {
            std::cout << '*';
        } else {
            std::cout << ch;
        }
        lastCharIsNl = (ch == '\n');
    }
    if (!lastCharIsNl) {
      std::cout << Colors::BG_WHITE << Colors::BLACK << '%' << Colors::RESET << std::endl;
    }
    file.close();
    std::cout << delimiterEnd << std::endl;
}

std::vector<std::string> readFileWithNewlines(const fs::path& filepath) {
  // Typical solution with getline has a problem disambiutating between a file
  // that ends with a newline and one that doesn't. This is a more granular method.
  std::ifstream file(filepath);
  std::vector<std::string> lines;
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filepath << std::endl;
    return lines;
  }

  std::string currentLine;
  char ch;
  while (file.get(ch)) {
    if (ch == '\n') {
      lines.push_back(currentLine);
      lines.push_back("\n");  // push newline as a separate string
      currentLine.clear();
    } else {
      currentLine += ch;
    }
  }
  if (!currentLine.empty() || (file.eof() && ch == '\n')) {
    lines.push_back(currentLine);
  }

  return lines;
}

std::pair<bool, std::string> diffFiles(const fs::path& file1, const fs::path& file2) {
  
  std::vector<std::string> lines1 = readFileWithNewlines(file1);
  std::vector<std::string> lines2 = readFileWithNewlines(file2);

  dtl::Diff<std::string> diff(lines1, lines2);
  diff.compose();
  dtl::Ses<std::string> ses = diff.getSes();

  std::string diffStr;
  bool isDifferent = false;

  for (const auto& sesElem : ses.getSequence()) {
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

void verboseDiffDump(const fs::path &testPath, const fs::path &expPath, const fs::path &genPath) {
  std::cout << "\nTestfile source Dump:\n";
  dumpFile(testPath);
  std::cout << "\n";
  std::cout << "Expected Output Dump:\n";
  dumpFile(expPath, true);
  std::cout << "\n";
  std::cout << "Generated Output Dump:\n";
  dumpFile(genPath, true); 
}

} // end anonymous namespace

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

  // if the test is an "error test" then the first lines follow a perscribed format.  
  std::string genErrorString = getErrorString(eo.getErrorFile());
  std::string expErrorString = getErrorString(test->getOutPath());

  if (eo.getReturnValue() != 0 && !genErrorString.empty() && !expErrorString.empty()) { 
    // Error test that propogated here since allowError is true in the config. 
    if (genErrorString == expErrorString) {
      return TestResult(testPath, true, true, "");
    }
    if (cfg.isVerbose()) {
      verboseDiffDump(test->getTestPath(), test->getOutPath(), eo.getErrorFile());  
    }
    return TestResult(testPath, false, true, "");

  } else {
    // Not an error test. Match the generated and expected outputs with exact diff.
    std::pair<bool, std::string> diff = diffFiles(eo.getOutputFile(), test->getOutPath());
    if (diff.first) { 
      if (cfg.isVerbose()) {
        verboseDiffDump(test->getTestPath(), test->getOutPath(), eo.getErrorFile()); 
      }
      return TestResult(test->getTestPath(), false, false, ""); 
    }
    return TestResult(test->getTestPath(), true, false, "");
  }
}

} // End namespace tester