#include "testharness/TestHarness.h"

#include "CLI11.hpp"

#include <iostream>
#include <fstream>
#include <exception>

int main(int argc, char **argv) {
  CLI::App app{"CMPUT 415 testing utility"};

  // Add the internal config file path option.
  std::string configFilePath;
  app.add_option("configFile", configFilePath, "Path to the tester JSON configuration file.")
    ->required()->check(CLI::ExistingFile);

  // Add quiet flag to not print out diffs.
  bool quiet;
  app.add_flag("-q,--quiet", quiet, "Quiet mode, don't print fail diffs");

  std::string summaryFilePath;
  app.add_option("--summary", summaryFilePath, "Write the test summary to this file instead of"
                                               "stdout");

  // Parse our command line options.
  CLI11_PARSE(app, argc, argv);

  // Open and read our json config file.
  std::ifstream jsonFile(configFilePath);
  JSON json;
  jsonFile >> json;

  try {
    // Build our tester.
    tester::TestHarness t(json, quiet);
    std::cout << t.getTestInfo() << '\n';

    // Run our tests.
    t.runTests();

    // Save or print the summary.
    std::string summary = t.getTestSummary();
    if (!summaryFilePath.empty()) {
      std::ofstream sumFile(summaryFilePath);
      sumFile << summary;
      std::cout << "Summary saved to file: " << summaryFilePath << '\n';
    }
    else {
      std::cout << "Summary:\n" << summary;
    }
  }
  catch (const std::runtime_error &e) {
    std::cout << "Test harness error: " << e.what() << '\n';
  }

  return 0;
}
