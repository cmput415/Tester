#ifndef TESTER_GRADER_H
#define TESTER_GRADER_H

#include "config/Config.h"
#include "tests/Util.h"
#include "json.hpp"

#include <ostream>
#include <string>
#include <utility>
#include <map>

using JSON = nlohmann::json;

namespace tester {

class Grader {
public:
  // No default constructor.
  Grader() = delete;

  // Construct with output file path.
  explicit Grader(const Config &cfg);

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

  // Our tests.
  TestModule tests;

  // The filtered (must have exe and tests) names of all solutions that will be tested.
  std::vector<std::string> names;

  // Tester tournament results
  JSON outputJson;

};

} // End namespace tester

#endif //TESTER_GRADER_H