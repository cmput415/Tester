#include "analysis/Grader.h"
#include "config/Config.h"
#include "testharness/TestHarness.h"

#include <exception>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {

  // Build the config and exit if it fails.
  tester::Config cfg(argc, argv);
  if (!cfg.isInitialised())
    return cfg.getErrorCode();

  // Grading means we don't run the tests like normal. Break early.
  const std::optional<fs::path>& gradePath = cfg.getGradePath();
  if (gradePath.has_value()) {
    tester::Grader grader(cfg);
    std::ofstream jsonOutput(*gradePath);
    grader.dump(jsonOutput);
    return 0;
  }

  bool failed = false;
  try {
    // Build our tester.
    tester::TestHarness t(cfg);

    std::cout << t.getTestInfo() << std::endl;

    // Run our tests.
    failed = t.runTests();

    // Free resources
  } catch (const std::runtime_error& e) {
    std::cout << e.what() << '\n';
    return 1;
  }

  return failed ? 1 : 0;
}
