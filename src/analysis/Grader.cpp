#include "analysis/Grader.h"

#include "tests/testUtil.h"

#include <iostream>

// File static namespace.
namespace {
}

namespace tester {

Grader::Grader(const Config &cfg) : cfg(cfg), tests(), analysis() {
  findTests(cfg.getInDirPath(), cfg.getOutDirPath(), cfg.getInStrDirPath(), tests);
  buildResults();
  analyseResults();
}

void Grader::buildResults() {
  auto &counts = analysis.addTable<TestCountTable>("counts", "Test Counts");

  // Use this loop for multiple purposes. Create the test counts, but also build up the vector of
  // test package names that have executables (theoretically this should be all of them).
  for (const auto &testPackage : tests) {
    // First check if the name exists in the executable lists.
    std::string name = testPackage.first;
    if (!cfg.hasExecutable(name)) {
      std::cerr << "Test package (" << name << ") missing executable.\n";
      continue;
    }

    // Add list name to list that will be tested.
    names.emplace_back(name);

    // Add the test count to the table.
    size_t count = 0;
    for (const auto &subpackage : testPackage.second)
      count += subpackage.second.size();
    counts.addTestCount(name, count);
  }

  // Start running tests. Make a pass rate table for each toolchain.
  for (const auto &toolChain : cfg.getToolChains()) {
    // Table strings.
    std::string toolChainName = toolChain.first;
    std::string tableName = toolChainName + "PassRate";
    std::string tableTitle = "Pass Rate (" + toolChainName + ")";

    // Make our table.
    auto &passRate = analysis.addTable<TestPassRateTable>(tableName, tableTitle);
    passRate.reserve(names);
    passRates.emplace_back(passRate);

    // Get the toolchain and start running tests. Run over names twice since it's nxn.
    ToolChain tc = toolChain.second;
    for (const std::string &defender : names) {
      // Set up the tool chain with the defender's executable.
      tc.setTestedExecutable(cfg.getExecutablePath(defender));

      // Iterate over attackers.
      for (const std::string &attacker : names) {
        std::cout << toolChainName << '-' << attacker << '-' << defender << ':';
        // Iterate over subpackages and the contained tests from the attacker, tracking pass count.
        size_t passCount = 0;
        for (const auto &subpackages : tests[attacker]) {
          for (const auto &test : subpackages.second) {
            if (runTest(test, tc, true).pass)
              ++passCount;

            // Status showing. Flushing every iteration isn't "ideal" but 1) I like seeing progress
            // visually, 2) run time is dominated by the toolchain. Flushing doesn't hurt.
            std::cout << '.';
            std::cout.flush();
          }
        }
        std::cout << '\n';

        // Save the pass rate.
        passRate.addPassRate(defender, attacker, passCount, counts.getTestCount(attacker));
      }
    }
  }
}

void Grader::analyseResults() {
  // Make the summary table.
  auto &passSummary = analysis.addTable<TestSummaryTable>("summary", "Pass Rate Summary");
  passSummary.reserve(names);

  // Average over all toolchains. If there's only one then this table is slightly redundant, but
  // it's the thought that counts.
  for (const std::string &defender : names) {
    for (const std::string &attacker : names) {
      // Make our vector of nodes to average over.
      std::vector<CellRef> cells;

      // Put in each rate.
      for (const auto &rateTable : passRates) {
        cells.emplace_back(rateTable.get().getCrossCell(defender, attacker));
      }

      // Add to the summary.
      passSummary.addSummary(defender, attacker, cells);
    }
  }

  // Build attack table.
  auto &offense = analysis.addTable<OffensivePointsTable>("offensive","Offensive Points Summary");
  for (const std::string &attacker : names)
    // We're comparing against the defender's names.
    offense.addAttacker(attacker, passSummary.getAttackerRange(attacker),
                        passSummary.getDefenderNameRange());

  // Build defense table.
  auto &defense = analysis.addTable<DefensivePointsTable>("defensive", "Defensive Points Summary");
  for (const std::string &defender : names)
    // We're comparing against the attackers's names.
    defense.addDefender(defender, passSummary.getDefenderRange(defender),
                        passSummary.getAttackerNameRange());

  // Mock up the base coverage table.
//  auto &coverage = analysis.addTable<TestCoverageTable>("coverage", "Solution Coverage Summary");
//  for (const std::string &solution : names)
//    // This is just a dummy thing. These values will be filled in by the marker.
//    coverage.addName(solution);

  // Build the summary table.
  auto &pointSum = analysis.addTable<PointSummaryTable>("points", "Points Summary");
  for (const std::string &solution : names)
    // Don't put the solution into the point summary. That would be unfair.
    if (solution != "solution")
      pointSum.addSummary(solution, offense.getCellByName(solution),
                          defense.getCellByName(solution),
                          passSummary.getCrossCell(solution, solution));

  // Build the final summary table.
  auto &finalSum = analysis.addTable<FinalSummaryTable>("final", "Final Summary");
  for (const std::string &solution : names)
    // Don't put the solution into the final summary. It's impossible.
    if (solution != "solution")
      finalSum.addSummary(solution, pointSum.getSummary(solution), pointSum.getSummaryRange(),
                          passSummary.getCrossCell(solution, "solution"));
}

} // End namespace tester
