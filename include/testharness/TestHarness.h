#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "testharness/PathPair.h"
#include "testharness/TestResult.h"
#include "testharness/ResultManager.h"
#include "toolchain/ToolChain.h"

#include "json.hpp"

#include <string>
#include <vector>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// Test file typedefs.
typedef std::vector<PathPair> PathList;
typedef std::map<std::string, PathList> TestSet;
typedef std::map<std::string, TestSet> PackageSet;

// Class that manages finding tests and running them.
class TestHarness {
public:
  // No default constructor.
  TestHarness() = delete;

  // Construct the Tester with a parsed JSON file.
  TestHarness(const JSON &json, bool quiet);

  // Run the found tests.
  void runTests();

  // Get tests info.
  std::string getTestInfo() const;


private:
  void runTestsForToolChain(const ToolChain &toolChain);

private:
  // The executable to test.
  std::vector<fs::path> testedExecutables;

  // The tool chain to compile something to test.
  std::vector<ToolChain> toolchains;

  // The list of tests to test.
  PackageSet tests;

  // The results of the tests.
  ResultManager results;

  // Should we print diffs?
  bool quiet;
};

} // End namespace tester

#endif //TESTER_TESTER_H
