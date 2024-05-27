#include "analysis/Grader.h"

#include <iostream>

// File static namespace.
namespace {
}

namespace tester {

Grader::Grader(const Config &cfg) : cfg(cfg), tests(), results(JSON::array()) {
  fillModule(cfg.getTestDirPath(), tests);  
  buildResults();
  analyseResults();
}

void Grader::buildResults() {
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
  }

  // Start running tests. Make a pass rate table for each toolchain.
  for (const auto &toolChain : cfg.getToolChains()) {
    // Table strings.
    std::string toolChainName = toolChain.first;

    JSON toolChainJson = {
      {"toolchain", toolChain.first},
      {"results", JSON::array()} 
    };

    // Get the toolchain and start running tests. Run over names twice since it's nxn.
    ToolChain tc = toolChain.second;
    for (const std::string &defender : names) {
      // Set up the tool chain with the defender's executable.
      tc.setTestedExecutable(cfg.getExecutablePath(defender));

      if (cfg.hasRuntime(defender))
        tc.setTestedRuntime(cfg.getRuntimePath(defender));
      else
        tc.setTestedRuntime("");

      // Iterate over attackers.
      for (const std::string &attacker : names) {
        // std::cout << toolChainName << '-' << attacker << '-' << defender << ':';
        std::cout << "==== " << toolChainName << " "<< attacker << " (attacker) V.S " << defender << " (defender)"<< "\n";
        // Iterate over subpackages and the contained tests from the attacker, tracking pass count.
        
        JSON attackDefenseJson = {
          {"attacker", attacker},
          {"defender", defender},
          {"testCount", 0},
          {"passCount", 0},
          {"timings", JSON::array()}
        };

        size_t passCount = 0, testCount = 0;
        for (const auto &subpackages : tests[attacker]) {
          for (const auto &test : subpackages.second) {
            
            TestResult result = runTest(test, tc, cfg); 
            
            if (result.pass) {
              std::cout << ".";
              passCount++;
            } else if (result.error) {
              std::cout << "x";
            }
            std::cout.flush();
            testCount++;  
          }
        }

        attackDefenseJson["passCount"] = passCount;
        attackDefenseJson["testCount"] = testCount;
        toolChainJson["results"].push_back(attackDefenseJson);
        
        std::cout << '\n';
      }
    }

    results.push_back(toolChainJson);
  }
}

/**
 * Radicaly simplify...
 * ...
 * ..
 * .
 * .
 * ..
 * ...
 * ....
*/
void Grader::analyseResults() {

  // for (auto& attacker: names) {
  //   for (auto& defender: names) {

  //   }
  // }
  // // Make the summary tables.
  // auto &totalPassRate = analysis.addTable<TotalPassRateTable>("passSummary", "Pass Rate Summary");
  // auto &totalFailRate = analysis.addTable<TotalFailRateTable>("failSummary", "Fail Rate Summary");
  // totalPassRate.reserve(names);
  // totalFailRate.reserve(names);

  // // Average over all toolchains to get the pass rate for all tests. If there's only one then this
  // // table is slightly redundant, but it's the thought that counts. While we're doing this, also
  // // construct the complement table that will be used for offensive points.
  // for (const std::string &defender : names) {
  //   for (const std::string &attacker : names) {
  //     // Make our vector of nodes to average over.
  //     std::vector<CellRef> cells;

  //     // Put in each rate.
  //     for (const auto &rateTable : passRates) {
  //       cells.emplace_back(rateTable.get().getCrossCell(defender, attacker));
  //     }
    
  //     // Add to the summaries.
  //     totalPassRate.addPassRate(defender, attacker, cells);
  //     totalFailRate.addFailRate(defender, attacker, totalPassRate.getCrossCell(defender, attacker));
  //   }
  // }

  // std::cout << "Build Attack Table" << std::endl;
  // // Build attack table.
  // auto &offense = analysis.addTable<OffensivePointsTable>("offensive","Offensive Points Summary");
  // for (const std::string &attacker : names)
  //   // We're comparing against the defender's names.
  //   offense.addAttacker(attacker, totalFailRate.getAttackerRange(attacker),
  //                       totalFailRate.getDefenderNameRange());

  // std::cout << "Build Defense Table" << std::endl;
  // // Build defense table.
  // auto &defense = analysis.addTable<DefensivePointsTable>("defensive", "Defensive Points Summary");
  // for (const std::string &defender : names)
  //   // We're comparing against the attackers's names.
  //   defense.addDefender(defender, totalPassRate.getDefenderRange(defender),
  //                       totalPassRate.getAttackerNameRange());

  // // Mock up the coverage multiplier table.
  // auto &coverageMults = analysis.addTable<CoverageTable>("coverageMults", "Test Coverage");
  // for (const std::string &solution : names)
  //   // Solution doesn't have a coverage.
  //   if (solution != "solution")
  //     // This is just a dummy thing. These values will be filled in by the marker.
  //     coverageMults.addName(solution);

  // // Build coverage table.
  // auto &coverage = analysis.addTable<CoveragePointsTable>("coverage", "Coverage Points Summary");
  // for (const std::string &solution : names)
  //   // Solution doesn't have a coverage.
  //   if (solution != "solution")
  //     coverage.addCoverage(solution, coverageMults.getCoverage(solution),
  //                          totalPassRate.getAttackerRange(solution),
  //                          totalPassRate.getDefenderNameRange());

  // std::cout << "Build Summary Table" << std::endl;
  // // Build the summary table.
  // auto &pointSum = analysis.addTable<PointSummaryTable>("points", "Points Summary");
  // for (const std::string &solution : names)
  //   // Don't put the solution into the point summary. That would be unfair.
  //   if (solution != "solution")
  //     pointSum.addSummary(solution, offense.getCellByName(solution),
  //                         defense.getCellByName(solution),
  //                         totalPassRate.getCrossCell(solution, solution),
  //                         coverage.getCellByName(solution));

  // std::cout << "Build Final Summary Table" << std::endl;
  // // Build the final summary table.
  // // auto &finalSum = analysis.addTable<FinalSummaryTable>("final", "Final Summary");
  // // for (const std::string &solution : names)
  // //   // Don't put the solution into the final summary. It's impossible.
  // //   if (solution != "solution")
  // //     finalSum.addSummary(solution, pointSum.getSummary(solution), pointSum.getSummaryRange(),
  // //                         totalPassRate.getCrossCell(solution, "solution"));
  
  // std::cout << "Finished Final Summary Table" << std::endl;
}

} // End namespace tester
