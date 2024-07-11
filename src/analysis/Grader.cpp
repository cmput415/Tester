#include "analysis/Grader.h"
#include <algorithm>
#include <iostream>

namespace tester {

void Grader::buildResults() {

  outputJson["title"] = "415 Grades";
  outputJson["results"] = JSON::array();
  
  JSON testSummary = {
    {"packages", JSON::array()},   
    {"executables", JSON::array()}
  };

  for (const auto& exe : cfg.getExecutables()) {
    
    std::string exeName = exe.first;     
    defendingExes.push_back(exeName);   
    testSummary["executables"].push_back(exeName);
  }
 
  for (const auto& testPackage : testSet) {

    std::string packageName = testPackage.first;
    attackingTestPackages.push_back(packageName);

    int count = 0;
    for (const auto &subpackage : testPackage.second)
      count += subpackage.second.size();
    
    JSON packageSummary = {
      {"name", packageName},
      {"count", count}
    };  
    testSummary["packages"].push_back(packageSummary);
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

      // Find max string length of team name for formatting stdout  
      auto maxNameLength = static_cast<int>(std::max_element(
        attackingTestPackages.begin(), attackingTestPackages.end(),
        [](const std::string &a, const std::string &b) {
          return a < b;
        }
      )->size());

      // Iterate over attackers.
      for (const std::string& attacker : attackingTestPackages) {
        
        std::cout << "  " << std::left << std::setw(maxNameLength + 2) << ("(" + attacker + ")") // +2 for the parentheses
          << " --> "
          << std::left << std::setw(maxNameLength + 2) << ("(" + defender + ")");
        
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
            JSON timingData = {
              {"test", test->getTestPath().filename()},
              {"time", test->getElapsedTime()},
              {"pass", result.pass}
            };
            attackResults["timings"].push_back(timingData);
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