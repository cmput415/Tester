#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "Colors.h"
#include "config/Config.h"
#include "testharness/ResultManager.h"
#include "tests/TestParser.h"
#include "tests/TestResult.h"
#include "toolchain/ToolChain.h"

#include <filesystem>
#include <map>
#include <string>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

// Test hierarchy types
typedef std::pair<std::unique_ptr<TestFile>, std::optional<TestResult>> TestPair;
typedef std::vector<TestPair> SubPackage;
typedef std::map<std::string, SubPackage> Package;
typedef std::map<std::string, Package> TestSet;

// Class that manages finding tests and running them.
class TestHarness {
public:
  // No default constructor.
  TestHarness() = delete;

  // Construct the Tester with a parsed JSON file.
  TestHarness(const Config& cfg) : cfg(cfg), results() { findTests(); }

  // Returns true if any tests failed, false otherwise.
  bool runTests();

  // Get tests info.
  std::string getTestInfo() const;

  // Get test summary.
  std::string getTestSummary() const;

protected:
  // JSON config 
  const Config& cfg;
  
  // where all the tests are held.
  TestSet testSet;

  // A separate subpackage, just for invalid tests.
  SubPackage invalidTests;

protected:
  // let derived classes find tests.
  void findTests();

private:
  // The results of the tests.
  //  NOTE we keep both a result manager and
  //  the result in the TestSet to ensure in-ordre
  //  printing
  ResultManager results;

private:
  // thread control
  void spawnThreads();

  // test running
  void threadRunTestBatch(std::reference_wrapper<std::vector<std::string>> toolchains,
                          std::reference_wrapper<std::vector<std::string>> executables,
                          std::reference_wrapper<std::vector<std::reference_wrapper<TestPair>>> tests,
                          std::reference_wrapper<size_t> currentIndex, std::reference_wrapper<std::mutex> lock);
  void threadRunTestsForToolChain(std::reference_wrapper<std::vector<std::string>> tcIds,
                                  std::reference_wrapper<std::vector<std::string>> exeNames,
                                  std::reference_wrapper<std::vector<std::reference_wrapper<TestPair>>> tests, size_t begin, size_t end);

  // helper for formatting tester output 
  void printTestResult(const TestFile *test, TestResult result);
  bool aggregateTestResultsForToolChain(std::string tcName, std::string exeName);

  // test finding and filling methods
  void addTestFileToSubPackage(SubPackage& subPackage, const fs::path& file);

  // utility for filling packages with tests
  void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath);
  void fillSubpackagesRecursive(Package& package, const fs::path& packPath,
                                const std::string& parentKey);
  void setupDebugModule(TestSet& testSet, const fs::path& debugPath);
};

} // End namespace tester

#endif // TESTER_TESTER_H
