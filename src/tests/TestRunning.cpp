#include "tests/TestRunning.h"

#include "Colors.h"
#include "config/Config.h"
#include "dtl/dtl.hpp"
#include "toolchain/CommandException.h"
#include "toolchain/ExecutionState.h"
#include <optional>
#include <fstream>
#include <sstream>
#include <tuple>

namespace {

/**
 * @brief Open up a file and print char by char to stdout. To increase the
 * visibility of spaces (which can cause sneaky diffs on testcases) we print
 * them as asterisks instead.
 */
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

/**
 * @brief Read a file character by character. Produce a vector of strings where
 * each string corresponds to a single line in the file until the newline and
 * each newline gets its own elmenet in the vector. This helps us differentiate
 * between two files where one only one is newline terminated by comparing the sizes
 * of the vectors.  
 */
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

/**
 * @brief a precise character by character diff between two files.
 * 
 * @param genFile file path with generated output of final toolchain step
 * @param expFile file path with expected output for the testcase. 
 * @returns a pair with 1) isDiff boolean and 2) diff string (empty if isDiff is false)
 */
std::pair<bool, std::string> preciseDiff(const fs::path& file1, const fs::path& file2) {

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
 * @brief: Given a file path, return the substring of the first line that conforms to
 * the error testcase specification.
 */
std::optional<std::string >getErrorString(const fs::path outPath) {

  std::ifstream ins(outPath); // open input file stream of output file of toolchain
  if (!ins.is_open()) {
    throw std::runtime_error("Failed to open the generated output file of the toolchain.");
  }

  std::string firstLine;
  if (!getline(ins, firstLine)) {
    // can't get the first line for some reason.
    return std::nullopt;
  }

  size_t errorStart = firstLine.find("Error");
  if (errorStart == std::string::npos) {
    // Error substring NEEDS to be in the first line somewhere.
    return std::nullopt;
  }

  std::string snipLHS = firstLine.substr(errorStart);
  // Generated outputs may have implementation defined message on the RHS of
  // a colon for an error output. If a colon exists, strip what it and what is on
  // the RHS of it.
  size_t colonPos = snipLHS.find(":");
  if (colonPos == std::string::npos) {
    return snipLHS;
  }
  // colon in output
  std::string snipRHS = snipLHS.substr(0, colonPos);
  return snipRHS;
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

/**
 * @brief custom diff implementation which corresponds to how we compare an error testcase
 * in the spec. Currently, we look for the first line in the generated output and match
 * if the expected output is a full substring, up to the first colon.
 * 
 * @param genFile path to the file containing generated output of final toolchain step
 * @param expFile path to the file containing expected output for the testcase. 
 * @returns pair indicating 1) success and 2) diff in case of failure
 */
std::pair<bool, std::string> errorDiff(const fs::path& genFile, const fs::path& expFile) {

  auto genErrorString = getErrorString(genFile);
  auto expErrorString = getErrorString(expFile);

  if ( genErrorString.has_value() && expErrorString.has_value()
                                 && *genErrorString == *expErrorString ) {
    return {false, ""};
  }

  std::string diffStr = ""; 
  return std::make_pair(true, std::move(diffStr));
} 

} // end anonymous namespace

namespace tester {

/**
 * @brief: Invoke the toolchain for the current test. Commands that exit with non-zero
 * throw inside the toolchain, causing an immediate fail unless the step is protected with an "allowError"
 * property. If "allowError" is true, then non-zero exits break the toolchain immediately and we check
 * the stderr of the command instead of stdout.
 */
TestResult runTest(TestFile* test, const ToolChain& toolChain, const Config& cfg) {

  const fs::path testPath = test->getTestPath();
  const fs::path expOutPath = test->getOutPath();
  const fs::path insPath = test->getInsPath(); 
  fs::path genOutPath;
  std::string genErrorString, expErrorString, diffString;
  
  // Track test results 
  bool testDiff = false, testError = false;
  std::pair<bool, std::string> testResult; 

  ExecutionOutput eo;
  try {
    eo = toolChain.build(test);

    // For error tests, we will use the stderr stream of the execution output.
    if (eo.IsErrorTest()) {
      genOutPath = eo.getErrorFile();
    } else {
      genOutPath = eo.getOutputFile();
    }

  } catch (const CommandException& ce) {
    // toolchain throws errors only when allowError is false in the config
    if (cfg.getVerbosity() > 0) {
      std::cout << Colors::YELLOW << "    [ERROR] " << Colors::RESET << ce.what() << '\n';
    }
    return TestResult(testPath, false, true, "");
  }
 
  // Make a precise diff and fallback to error diff if  the precise diff failed.
  testResult = preciseDiff(genOutPath, expOutPath);
  if (testResult.first) {
    testResult = errorDiff(genOutPath, expOutPath);   
  }

  // Unpack results
  std::tie(testDiff, diffString) = testResult;

  // if there is a diff in the output, pick the defined way to display it based on config.
  int verbosity = cfg.getVerbosity();
  if (verbosity == 3) {
    // highest level of verbosity results in printing the full output even for passing tests.
    formatFileDump(testPath, expOutPath, genOutPath);
  } else if (verbosity == 2 && testDiff) {
    // level two dump the relevant files
    formatFileDump(testPath, expOutPath, genOutPath);
  } else if (verbosity == 1 && testDiff) {
    // level one simply print the diff string
    std::cout << diffString << std::endl;
  }
  
  return TestResult(testPath, !testDiff, testError, "");
}

} // End namespace tester