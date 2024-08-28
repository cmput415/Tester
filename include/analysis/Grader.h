#ifndef TESTER_GRADER_H
#define TESTER_GRADER_H

#include "config/Config.h"
#include "json.hpp"
#include "testharness/TestHarness.h"
#include "tests/TestRunning.h"

#include <map>
#include <ostream>
#include <string>
#include <utility>

using JSON = nlohmann::json;

namespace tester {

class Grader : public TestHarness {
public:
  // No default constructor.
  Grader() = delete;

  // Construct with output file path.
  Grader(const Config& cfg) : TestHarness(cfg), 
                              failedTestLog(*cfg.getFailureLogPath()),
                              solutionExecutable(*cfg.getSolutionExecutable()) {
    findTests();
    buildResults();
  }

  void dump(std::ostream& os) const {
    std::string jsonString = outputJson.dump(2);
    os << jsonString;
  }

private:
  // Build the results to produce our sheet.
  void buildResults();
  
  // Track if a testcase makes the solution compiler fail. Record in a file
  void trackSolutionFailure(const TestFile *test, const std::string& toolchainName,
                                                  const std::string& attackingPackage);

private:
  std::ofstream failedTestLog;
  std::string solutionExecutable;

  // defenders and attackers (defending exes should be a strict subset of attacking packages) 
  std::vector<std::string> defendingExes;
  std::vector<std::string> attackingTestPackages;

  // Tester tournament results
  JSON outputJson;

private:
  // Helpers to factor out responsibility of buildResults
  void fillTestSummaryJSON();
  void fillToolchainResultsJSON();
};

} // End namespace tester

#endif // TESTER_GRADER_H