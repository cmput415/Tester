#ifndef TESTER_GRADER_H
#define TESTER_GRADER_H

#include "analysis/spreadsheet/Sheet.h"
#include "analysis/spreadsheet/Table.h"
#include "config/Config.h"
#include "tests/testUtil.h"

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

  void dump(std::ostream &os) const { analysis.dumpSV(os); }

private:
  // Build the results to produce our sheet.
  void buildResults();

  // Analyse the results to produce our sheet.
  void analyseResults();

private:
  // Our config.
  const Config &cfg;

  // Our tests.
  PackageSet tests;

  // The sheet that our analysis goes into.
  Sheet analysis;

  // The filtered (must have exe and tests) names of all solutions that will be tested.
  std::vector<std::string> names;

  // The vector of pass rate tables.
  typedef std::reference_wrapper<tester::TestPassRateTable> TestPassRateTableRef;
  std::vector<TestPassRateTableRef> passRates;
};

} // End namespace tester

#endif //TESTER_GRADER_H
