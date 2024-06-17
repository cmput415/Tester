#include "tests/TestRunning.h"

#include "Colors.h"
#include "config/Config.h"
#include "dtl/dtl.hpp"
#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"

#include <fstream>
#include <sstream>
#include <tuple>

namespace {

void dumpFile(const fs::path& filePath, bool showSpace = false) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filePath << std::endl;
    return;
  }
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
}

std::vector<std::string> readFileWithNewlines(const fs::path& filepath) {

  std::ifstream file(filepath);
  std::vector<std::string> lines;
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }

  std::string currentLine;
  char ch;
  while (file.get(ch)) {
    if (ch == '\n') {
      lines.push_back(currentLine);
      lines.push_back("\n"); // push newline as a separate string
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

std::pair<bool, std::string> getDiffString(const fs::path& file1, const fs::path& file2) {

  std::vector<std::string> lines1;
  std::vector<std::string> lines2;
  bool isDiff = false; // do the files have any difference

  try {
    lines1 = readFileWithNewlines(file1);
    lines2 = readFileWithNewlines(file2);
  } catch (const std::runtime_error& e) {
    return std::make_pair(false, "");
  }

  dtl::Diff<std::string> diff(lines1, lines2);
  diff.compose();
  dtl::Ses<std::string> ses = diff.getSes();

  std::string diffStr = std::string();
  for (const auto& sesElem : ses.getSequence()) {
    if (sesElem.second.type != dtl::SES_COMMON) {
      if (sesElem.second.type == dtl::SES_ADD) {
        diffStr += Colors::RED + "-" + sesElem.first + Colors::RESET + "\n";
      } else {
        diffStr += Colors::GREEN + "+" + sesElem.first + Colors::RESET + "\n";
      }
      isDiff = true;
    }
  }
  return std::make_pair(isDiff, std::move(diffStr));
}

/**
 * Given a file path, return the substring of the first line that conforms to
 * the error testcase specification.
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

void formatFileDump(const fs::path& testPath, const fs::path& expOutPath,
                    const fs::path& genOutPath) {
  std::cout << "----- TestFile: "<< testPath.filename() << std::endl;
  dumpFile(testPath);
  std::cout << "----- Expected Output (" << fs::file_size(expOutPath) << " bytes)" << std::endl;
  dumpFile(expOutPath, true);
  std::cout << "----- Generated Output (" << fs::file_size(genOutPath) << " bytes)" << std::endl;
  dumpFile(genOutPath, true);
  std::cout << "-----------------------" << std::endl;
}

} // end anonymous namespace

namespace tester {

TestResult runTest(TestFile* test, const ToolChain& toolChain, const Config& cfg) {

  ExecutionOutput eo("");
  const fs::path testPath = test->getTestPath();
  const fs::path expOutPath = test->getOutPath();
  const fs::path insPath = test->getInsPath();
  fs::path genOutPath;
  std::string genErrorString, expErrorString, diffString;

  try {
    eo = toolChain.build(test);
    genOutPath = eo.getErrorFile();
    genErrorString = getErrorString(eo.getErrorFile());
    expErrorString = getErrorString(test->getOutPath());

  } catch (const CommandException& ce) {
    // toolchain throws errors only when allowError is false in the config
    if (cfg.getVerbosity() > 0) {
      std::cout << Colors::YELLOW << "    [ERROR] " << Colors::RESET << ce.what() << '\n';
    }
    return TestResult(testPath, false, true, "");
  }

  bool outputDiff = false;
  bool testError = false;

  if (eo.getReturnValue() != 0 && !genErrorString.empty() && !expErrorString.empty()) {
    outputDiff = (genErrorString == expErrorString) ? false : true;
    testError = true;
  } else {
    auto diffPair = getDiffString(genOutPath, expOutPath);
    outputDiff = diffPair.first;  // is there a difference between expected and generated
    diffString = diffPair.second; // the diff string
  }

  // if there is a diff in the output, pick the defined way to display it.
  int verbosity = cfg.getVerbosity();
  if (verbosity == 3) {
    // highest level of verbosity results in printing the full output even for passing tests.
    formatFileDump(testPath, expOutPath, genOutPath);
  } else if (verbosity == 2 && outputDiff) {
    // level two dump the relevant files
    formatFileDump(testPath, expOutPath, genOutPath);
  } else if (verbosity == 1 && outputDiff) {
    // level one simply print the diff string
    std::cout << diffString << std::endl;
  }

  return TestResult(testPath, !outputDiff, testError, "");
}

} // End namespace tester