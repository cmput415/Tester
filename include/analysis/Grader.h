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

// typedef std::map<std::string, std::string>    

class Grader {
public:
  // No default constructor.
  Grader() = delete;

  // Construct with output file path.
  explicit Grader(const Config &cfg);

  void dump(std::ostream &os) const {
    std::string jsonString = results.dump(2);
    os << jsonString;
  }

private:
  // Build the results to produce our sheet.
  void buildResults();

  // Analyse the results to produce our sheet.
  void analyseResults();

private:
  // Our config.
  const Config &cfg;

  // Our tests.
  TestModule tests;

  // The filtered (must have exe and tests) names of all solutions that will be tested.
  std::vector<std::string> names;

  // JSON object
  JSON results;

};

} // End namespace tester

#endif //TESTER_GRADER_H



/**
The Vision:
[
  {
    "toolchain": "arm",
    "results": {
      {
        "atacker": "team1",
        "defender": "team1",
        "testCount": 3,
        "testPassed": 3,
        "timings (ms)": [10.6, 90.3, 8.5]
      },
      {
        "atacker": "team1",
        "defender": "team2",
        "testCount": 3,
        "testPassed": 1,
        "timings (ms)": [10.6, -1, -1]
      },
      {
        "atacker": "team1",
        "defender": "team1",
        "testCount": 3,
        "testPassed": 1,
        "timings (ms)": [-1, 4, -1]
      },
      {
        "atacker": "team2",
        "defender": "team1",
        "testCount": 50,
        "testPassed": 48,
        "timings (ms)": [10.6, 90.3, 8.5...............]
      },
      ......
    }  
  }, {
  "toolchain": "riscv",
  "results": {
    {
      "atacker": "team1",
      "defender": "team1",
      "testCount": 3,
      "testPassed": 3,
      "timings (ms)": [10.6, 90.3, 8.5]
    },
    {
      "atacker": "team1",
      "defender": "team2",
      "testCount": 3,
      "testPassed": 1,
      "timings (ms)": [10.6, -1, -1]
    },
    {
      "atacker": "team1",
      "defender": "team1",
      "testCount": 3,
      "testPassed": 1,
      "timings (ms)": [-1, 4, -1]
    },
    {
      "atacker": "team2",
      "defender": "team1",
      "testCount": 50,
      "testPassed": 48,
      "timings (ms)": [10.6, 90.3, 8.5...............]
    },
    ......
  }
  }, {
    "toolchain": "riscv",
    "results": {
      {
        "atacker": "team1",
        "defender": "team1",
        "testCount": 3,
        "testPassed": 3,
        "timings (ms)": [10.6, 90.3, 8.5]
      },
      {
        "atacker": "team1",
        "defender": "team2",
        "testCount": 3,
        "testPassed": 1,
        "timings (ms)": [10.6, -1, -1]
      },
      {
        "atacker": "team1",
        "defender": "team1",
        "testCount": 3,
        "testPassed": 1,
        "timings (ms)": [-1, 4, -1]
      },
      {
        "atacker": "team2",
        "defender": "team1",
        "testCount": 50,
        "testPassed": 48,
        "timings (ms)": [10.6, 90.3, 8.5...............]
      },
      ...... 
    }
  }
]
*/