#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "config/Config.h"
#include "tests/testUtil.h"
#include "tests/TestModule.h"
#include "testharness/ResultManager.h"
#include "toolchain/ToolChain.h"

#include <string>
#include <map>
#include <filesystem>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

// Class that manages finding tests and running them.
class TestHarness {
public:
  // No default constructor.
  TestHarness() = delete;

  // Construct the Tester with a parsed JSON file.
  TestHarness(const Config &cfg);

  // Run the found tests.
  // Returns true if any tests failed, false otherwise.
  bool runTests();

  // Get tests info.
  std::string getTestInfo() const;

  // Get test summary.
  std::string getTestSummary() const;

private:
  // Runs the accumulated tests against a specific exe and toolchain.
  // Returns true if any tests failed, false otherwise.
  bool runTestsForToolChain(std::string tcId, std::string exeName);

private:
  // Our input config.
  const Config &cfg;

  // The results of the tests.
  ResultManager results;
  
  // File Check Test Module
  TestModule module;
};

} // End namespace tester

#endif //TESTER_TESTER_H
