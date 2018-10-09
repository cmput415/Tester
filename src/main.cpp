#include "config/Config.h"
#include "analysis/Grader.h"
#include "testharness/TestHarness.h"

#include <iostream>
#include <fstream>
#include <exception>

int main(int argc, char **argv) {
  // Build the config and exit if it fails.
  tester::Config cfg(argc, argv);
  if (!cfg.isInitialised())
    return cfg.getErrorCode();

  // Grading means we don't run the tests like normal. Break early.
  if (cfg.hasGradePath()) {
    tester::Grader grader(cfg);
    std::ofstream svFile(cfg.getGradePath());
    grader.dump(svFile);
    return 0;
  }

  try {
    // Build our tester.
    tester::TestHarness t(cfg);
    std::cout << t.getTestInfo() << '\n';

    // Run our tests.
    t.runTests();

    // Save or print the summary.
    std::string summary = t.getTestSummary();
    if (cfg.hasSummaryPath()) {
      std::ofstream sumFile(cfg.getSummaryPath());
      sumFile << summary;
      std::cout << "Summary saved to file: " << cfg.getSummaryPath() << '\n';
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
