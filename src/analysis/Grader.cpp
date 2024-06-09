#include "analysis/Grader.h"
#include <algorithm>
#include <iostream>

// File static namespace.
namespace {

bool containsString(const std::vector<std::string>& vec, const std::string& str) {
  return std::find(vec.begin(), vec.end(), str) != vec.end();
}

}

namespace tester {

void Grader::buildResults() {

  outputJson["title"] = "415 Grades";
  outputJson["results"] = JSON::array();
  
  // infer the name of each time from the executable string
  for (const auto& exe : cfg.getExecutables()) {
    std::cout << "exe: " << exe.first << std::endl;
    defendingExes.push_back(exe.first);   
  }

  // build a preliminary test summary object
  JSON testSummary = JSON::array();
  attackingTestPackages = defendingExes; // start with a copy to preserve symmetric ordering
  for (const auto& testPackage : testSet) {

    std::string packageName = testPackage.first;

    if (!containsString(defendingExes, packageName)) {
      attackingTestPackages.push_back(packageName);
    } 

    // Create the summary item.
    JSON summaryItem = {{"team", packageName}};
    size_t count = 0;
    for (const auto& subpackage : testPackage.second) {
      count += subpackage.second.size();
    }
    summaryItem["testCount"] = count;
    testSummary.push_back(summaryItem);
  }

  outputJson["testSummary"] = testSummary;

  // Start running tests. Make a pass rate table for each toolchain.
  for (const auto& toolChain : cfg.getToolChains()) {

    // Table strings.
    std::string toolChainName = toolChain.first;
    JSON toolChainJson = {{"toolchain", toolChain.first}, {"toolchainResults", JSON::array()}};
    std::cout << "Toolchain: " << toolChain.first << std::endl;

    // Get the toolchain and start running tests. Run over names twice since it's nxn.
    ToolChain tc = toolChain.second;
    for (const std::string& defender : defendingExes) {

      JSON defenseResults = {{"defender", defender}, {"defenderResults", JSON::array()}};
      // Set up the tool chain with the defender's executable.
      tc.setTestedExecutable(cfg.getExecutablePath(defender));

      if (cfg.hasRuntime(defender))
        tc.setTestedRuntime(cfg.getRuntimePath(defender));
      else
        tc.setTestedRuntime("");

      // Iterate over attackers.
      int maxNameLength = 20, arrowStart = 10;
      for (const std::string& attacker : attackingTestPackages) {
        std::cout << "  " << std::setw(arrowStart) << std::left << "(" + attacker + ")"
          << std::setw(maxNameLength - arrowStart) << " --> "
          << std::setw(10) << "(" + defender + ") ";
        
        JSON attackResults = {{"attacker", attacker}, {"timings", JSON::array()}};

        // Iterate over subpackages and the contained tests from the
        // attacker, tracking pass count.
        size_t passCount = 0, testCount = 0;
        for (const auto& subpackages : testSet[attacker]) {
          for (const std::unique_ptr<TestFile>& test : subpackages.second) {

            TestResult result = runTest(test.get(), tc, cfg);

            if (result.pass && !result.error) {
              // a regular test that passes
              std::cout << Colors::GREEN << "." << Colors::RESET;
              passCount++;
            } else if (result.pass && result.error) {
              // a test failed due to error but in the expected manner
              std::cout << Colors::GREEN << "x" << Colors::RESET; 
              passCount++;
            } else if (!result.pass && result.error) {
              // a test failed due to error unexpectedly
              std::cout << Colors::RED << "x" << Colors::RESET;
            } else {
              // a test that produced a different output
              std::cout << Colors::RED << "." << Colors::RESET;
            }
            std::cout.flush();
            testCount++;
            attackResults["timings"].push_back({
              test->getTestPath().filename(),
              test->getElapsedTime()
            });
          }
        }
        // update the test results
        attackResults["passCount"] = passCount;
        attackResults["testCount"] = testCount;
        defenseResults["defenderResults"].push_back(attackResults);

        std::cout << '\n';
      }
      // add the defense results
      toolChainJson["toolchainResults"].push_back(defenseResults);
    }
    // add the results for the entire toolchain
    outputJson["results"].push_back(toolChainJson);
  }
}

} // End namespace tester