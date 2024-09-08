#include "analysis/Grader.h"
#include <algorithm>
#include <iostream>

namespace {

/// @brief Print the output of a grade test in a way that gives a sense as to 
/// the overall direction of the tournament results to the terminal viewer.
/// Tests that pass on stdout are green dots, failures are red dots.
/// Error tests that pass are a green 'X', failures are red 'X'
void printGraderTestResult(bool testPass, bool testError) {
  if (testPass && !testError) {
    // a regular test that passes
    std::cout << Colors::GREEN << "." << Colors::RESET;
    // passCount++;
  } else if (testPass && testError) {
    // a test failed due to error but in the expected manner
    std::cout << Colors::GREEN << "x" << Colors::RESET; 
    // passCount++;
  } else if (testPass && testError) {
    // a test failed due to error unexpectedly
    std::cout << Colors::RED << "x" << Colors::RESET;
  } else {
    // a test that produced a different output
    std::cout << Colors::RED << "." << Colors::RESET;
  }
}

} // end anonymous namespace

namespace tester {

/// Check if the defender is the solution and a test has caused it to fail.
/// In this case we should identify and flag the testcase. For Generator, SCalc and VCalc
/// the solutions are closed, hence an failing test implies the test must be invalid.
/// For Gazprea a test which causes the soluition compiler to fail is more complicated. 
/// Either way we can collect all such test-cases for the TA to review.
void Grader::trackSolutionFailure(const TestFile *test, 
                                  const std::string& toolchainName,
                                  const std::string& attackingPackage) {
  // log the current toolchain 
  failedTestLog << toolchainName << " ";
  
  // log the offending attacker package
  failedTestLog << attackingPackage << " ";

  // log the path of the 
  fs::path testPath = fs::absolute(test->getTestPath()).lexically_normal();
  failedTestLog << testPath.string() << std::endl;
}

void Grader::fillTestSummaryJSON() {
  
  JSON testSummary = {
    {"packages", JSON::array()},   
    {"executables", JSON::array()}
  };

  // The results JSON records all the executables being run in the tournament
  for (const auto& exe : cfg.getExecutables()) {   
    std::string exeName = exe.first;     
    defendingExes.push_back(exeName);   
    testSummary["executables"].push_back(exeName);
  }

  // We also track all the test packages being run, along with their count
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
}

void Grader::fillToolchainResultsJSON() {

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
          for (const TestPair& testpair : subpackages.second) {
            const std::unique_ptr<TestFile>& test = testpair.first;

            TestResult result = runTest(test.get(), tc, cfg);
            
            if (!result.pass && defender == solutionExecutable) {
              if ( attacker == solutionExecutable ) {
                // A testcase just failed the solution executable
                throw std::runtime_error("A solution test just made the solution compiler fail!");
              }
              trackSolutionFailure(test.get(), toolChain.first, attacker);
            }
            //  
            if (result.pass) {
              testCount++;
            }
            // Print the test result in a nice to read format.
            printGraderTestResult(result.pass, result.error);

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

void Grader::buildResults() {

  outputJson["title"] = "415 Grades";
  outputJson["results"] = JSON::array();

  fillTestSummaryJSON();
  fillToolchainResultsJSON();
}

} // End namespace tester
