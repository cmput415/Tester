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
  Grader(const Config& cfg) : TestHarness(cfg) {
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

private:
  // The filtered (must have exe and tests) names of all solutions to be tested
  // std::vector<std::string> defenderNames;
  
  std::vector<std::string> defendingExes;
  std::vector<std::string> attackingTestPackages;

  // Tester tournament results
  JSON outputJson;
};

} // End namespace tester

#endif // TESTER_GRADER_H