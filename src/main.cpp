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

  bool quiet;
  app.add_flag("-q,--quiet", quiet, "Quiet mode, don't print fail diffs");

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

//      return 0;
    // Run our tests.
    t.runTests();
  }
  catch (const std::runtime_error &e) {
    std::cout << "Test harness error: " << e.what() << '\n';
  }

  return 0;
}
