#ifndef TESTER_GRADER_H
#define TESTER_GRADER_H

#include "config/Config.h"
#include "tests/Util.h"
#include "json.hpp"
#include "testharness/TestHarness.h"

#include <ostream>
#include <string>
#include <utility>
#include <map>

using JSON = nlohmann::json;

namespace tester {

class Grader : public TestHarness {
public:
  // No default constructor.
  Grader() = delete;

  // Construct with output file path.
  Grader(const Config &cfg) : TestHarness(cfg), cfg(cfg) { findTests(); buildResults(); }

  void dump(std::ostream &os) const {
    std::string jsonString = outputJson.dump(2);
    os << jsonString;
  }

private:
  // Build the results to produce our sheet.
  void buildResults();

private:
  // Our config.
  const Config &cfg;
  
  // The filtered (must have exe and tests) names of all solutions that will be tested.
  std::vector<std::string> names;

  // Tester tournament results
  JSON outputJson;

};

} // End namespace tester

#endif //TESTER_GRADER_H