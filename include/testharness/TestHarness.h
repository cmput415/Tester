#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "tests/testUtil.h"
#include "testharness/ResultManager.h"
#include "toolchain/ToolChain.h"

#include "json.hpp"

#include <string>
#include <map>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;
namespace fs = std::experimental::filesystem;

namespace tester {

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

  // Get test summary.
  std::string getTestSummary() const;

private:
  // Runs the accumulated tests against a specific exe and toolchain.
  void runTestsForToolChain(std::string tcId, std::string exeName);

private:
  // The executable to test.
  std::map<std::string, fs::path> executables;

  // The tool chain to compile something to test.
  std::map<std::string, ToolChain> toolchains;

  // The list of tests to test.
  PackageSet tests;

  // The results of the tests.
  ResultManager results;

  // Should we print diffs?
  bool quiet;
};

} // End namespace tester

#endif //TESTER_TESTER_H
