#ifndef TESTER_GRADER_H
#define TESTER_GRADER_H

#include "config/Config.h"
#include "tests/Util.h"
#include "csv/csv.h"

#include <ostream>
#include <string>
#include <utility>

namespace tester {

class Grader {
public:
  // No default constructor.
  Grader() = delete;

  // Construct with output file path.
  explicit Grader(const Config &cfg);

  // void dump(std::ostream &os) const { analysis.dumpSV(os); }

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
};

} // End namespace tester

#endif //TESTER_GRADER_H
