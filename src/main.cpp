#include "config/Config.h"
#include "testharness/TestHarness.h"

#include <iostream>
#include <fstream>
#include <exception>

int main(int argc, char **argv) {
  tester::Config cfg(argc, argv);
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
