#include "analysis/Grader.h"

#include <iostream>

// File static namespace.
namespace {
}

namespace tester {

void Grader::buildResults() {
  
  // outputJson["toolchainCount"] = cfg.getToolChains().size();
  outputJson["title"] = "415 Grades";
  outputJson["results"] = JSON::array();
  
  // collect summary of grading
  JSON testSummary = JSON::array(); 
  for (const auto &testPackage : testSet) {
    // First check if the name exists in the executable lists.
    std::string name = testPackage.first;
    names.push_back(name);
    if (!cfg.hasExecutable(name)) {
      std::cerr << "Test package (" << name << ") missing executable.\n";
      continue;
    }

    // Create the summary item.
    JSON summaryItem = { {"team", name} }; 
    size_t count = 0;
    for (const auto &subpackage : testPackage.second) {
      count += subpackage.second.size();
    }
    summaryItem["testCount"] = count;
    testSummary.push_back(summaryItem);
  }

  outputJson["testSummary"] = testSummary;

  // Start running tests. Make a pass rate table for each toolchain.
  for (const auto &toolChain : cfg.getToolChains()) {

    // Table strings.
    std::string toolChainName = toolChain.first;
    JSON toolChainJson = {
      {"toolchain", toolChain.first},
      {"toolchainResults", JSON::array()} 
    };

    // Get the toolchain and start running tests. Run over names twice since it's nxn.
    ToolChain tc = toolChain.second;
    for (const std::string &defender : names) {

      JSON defenseResults = {
        {"defender", defender},
        {"defenderResults", JSON::array()}
      };
      // Set up the tool chain with the defender's executable.
      tc.setTestedExecutable(cfg.getExecutablePath(defender));

      if (cfg.hasRuntime(defender))
        tc.setTestedRuntime(cfg.getRuntimePath(defender));
      else
        tc.setTestedRuntime("");

      // Iterate over attackers.
      for (const std::string &attacker : names) {
        std::cout << "==== " << toolChainName << " "<< attacker << " (attacker) V.S " << defender << " (defender)"<< "\n";

        JSON attackResults = {
          {"attacker", attacker},
          {"timings", JSON::array()}
        }; 

        // Iterate over subpackages and the contained tests from the attacker, tracking pass count.
        size_t passCount = 0, testCount = 0;
        for (const auto &subpackages : testSet[attacker]) {
          for (const std::unique_ptr<TestFile> &test : subpackages.second) {
            
            TestResult result = runTest(test.get(), tc, cfg); 
            
            if (result.pass) {
              std::cout << ".";
              passCount++;
            } else if (result.error) {
              std::cout << "x";
            }
            std::cout.flush();
            testCount++;

            attackResults["timings"].push_back({test->getTestPath(), test->getElapsedTime()});  
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