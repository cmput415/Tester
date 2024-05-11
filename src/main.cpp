#include "config/Config.h"
#include "analysis/Grader.h"
#include "testharness/TestHarness.h"

#include <iostream>
#include <fstream>
#include <exception>
#define DEBUG

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

  bool failed = false;
  try {
    // Build our tester.
    tester::TestHarness t(cfg);

    #ifdef DEBUG      
      std::cout << t.getTestInfo() << std::endl;
      tester::PathMap exePaths = cfg.getExecutables();
      tester::PathMap rtPaths = cfg.getRuntimes();
      for (auto exe : exePaths) {
        std::cout << exe.first << exe.second << std::endl;
      }
      for (auto runtime: rtPaths) {
        std::cout << runtime.first << runtime.second << std::endl;
      }
      for (const auto &tcPair : cfg.getToolChains()) {
        std::cout << tcPair.first << std::endl;
      }
    #endif

    // Run our tests.
    failed = t.runTests();

    #ifdef DEBUG
      return 0;
    #endif

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
    return 1;
  }

  return failed ? 1 : 0;
}
