#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "testharness/TestPair.h"
#include "testharness/TestResult.h"
#include "testharness/ResultManager.h"
#include "toolchain/ToolChain.h"

#include "json.hpp"

#include <string>
#include <vector>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// Test file typedefs.
typedef std::vector<TestPair> TestList;
typedef std::map<std::string, TestList> TestSet;

// Class that manages finding tests and running them.
class TestHarness {
public:
  // No default constructor.
  TestHarness() = delete;

  // Construct the Tester with a parsed JSON file.
  explicit TestHarness(const JSON &json);

  // Run the found tests.
  void runTests();

  // Get tests info.
  std::string getTestInfo() const;

private:
  // Runs a single test. True/false based on test pass.
  TestResult runTest(const TestPair &tp) const;

private:
  ToolChain toolchain;
  TestSet tests;
  ResultManager results;
};

} // End namespace tester

#endif //TESTER_TESTER_H
