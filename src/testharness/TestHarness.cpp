#include "testharness/TestHarness.h"

#include "tests/TestResult.h"

#include "util.h"

#include <sstream>
#include <utility>
#include <iostream>
#include <filesystem>

namespace tester {

TestHarness::TestHarness(const Config &cfg) : cfg(cfg), results(), module() {
  // Build the test set.
  fillModule(cfg.getTestDirPath(), module);
}

bool TestHarness::runTests() {
  bool failed = false;
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto &tcPair : cfg.getToolChains()) {
      if (runTestsForToolChain(exePair.first, tcPair.first) == 1)
        failed = true;
    }
  }
  return failed;
}

std::string TestHarness::getTestInfo() const {
  std::string rv = "Tests:\n";
  for (auto &package : module) {
    rv += "  " + package.first + ": " + std::to_string(package.second.size()) + '\n';
  }
  return rv;
}

std::string TestHarness::getTestSummary() const {
  std::stringstream allInfo;

  // Iterate over executables.
  for (const auto &exePair : cfg.getExecutables()) {
    // Write out this executables header.
    allInfo << exePair.first << ":\n";

    // The streams for package info. Self is for an exectuable's own test package, other for the
    // others.
    std::stringstream selfPackageInfo;
    std::stringstream otherPackageInfo;

    // We leave the above streams blank because there might not be tests in that category. These
    // bools track if we've written to them. If they're false and you're about to write to it then
    // you should add an appropriate header.
    bool toSelf = false, toOther = false;

    // Iterate over the test packages.
    for (const auto &package : module) {
      // Success count.
      unsigned int passes = 0, count = 0;

      // Iterate over toolchains.
      for (const auto &tcPair : cfg.getToolChains()) {

        std::cout << "toolchain pair" << std::endl;
        std::cout << tcPair.first << tcPair.second << std::endl;

        // Get the list of results for this exe, toolchain, and package.
        const ResultList &packageResults =
          results.getResults(exePair.first, tcPair.first).at(package.first);

        // Iterate over the results.
        for (const auto &result : packageResults) {
          if (result.pass) {
            assert(!result.error && "Test passed with error.");
            ++passes;
          }
        }

        // Save the size.
        count += packageResults.size();
      }

      // If this executable is linked with this package then it's the executable maker's set of
      // tests. We handle this slightly differently.
      if (exePair.first == package.first) {
        // We should only ever write to the self stream once, so toSelf is more of a trap than a
        // flag. We include the header in the one write as well.
        assert(!toSelf && "Already written to self");
        toSelf = true;
        selfPackageInfo << "  Self tests:\n    " << package.first << ": " << passes << " / " << count
                        << " -> " << (passes == count ? "PASS" : "FAIL") << '\n';
      }
      else {
        // Add other header.
        if (!toOther) {
          otherPackageInfo << "  Other tests:\n";
          toOther = true;
        }

        // Write info.
        otherPackageInfo << "    " << package.first << ": " << passes << " / " << count << '\n';
      }
    }

    allInfo << selfPackageInfo.str() << otherPackageInfo.str();
  }

  return allInfo.str();
}

bool TestHarness::runTestsForToolChain(std::string exeName, std::string tcName) {

  bool failed = false;

  // Get the toolchain to use.
  ToolChain toolChain = cfg.getToolChain(tcName);

  // Set the toolchain's exe to be tested.
  const fs::path &exe = cfg.getExecutablePath(exeName);
  std::cout << "\nTesting executable: " << exeName << " -> " << exe << '\n';
  toolChain.setTestedExecutable(exe);

  // If we have a runtime, set that as well.
  if (cfg.hasRuntime(exeName))
    toolChain.setTestedRuntime(cfg.getRuntimePath(exeName));
  else
    toolChain.setTestedRuntime("");

  // Say which toolchain.
  std::cout << "With toolchain: " << tcName << " -> " <<  toolChain.getBriefDescription() << '\n';

  // Stat tracking for toolchain tests.
  unsigned int toolChainCount = 0, toolChainPasses = 0;

  // Iterate the module
  for (const auto& [pKey, package] : module) {
    std::cout << "Entering package: " << pKey << '\n';
    
    uint32_t packageCount = 0, packagePasses = 0;
    
    for (const auto& [spKey, subPackage] : module[pKey]) {
      std::cout << "Entering SubPackage: " << spKey << std::endl;
      
      uint32_t subPackagePasses = 0;
      packageCount += subPackage.size(); 
      
      for (const auto& test : subPackage) {  
        TestResult result = runTest(test, toolChain, cfg.isQuiet());
        results.addResult(exeName, tcName, pKey, result);
        // Log the pass/fail.
        std::cout << "    " << test->testPath.filename().string() << ": "
                  << (result.pass ? "PASS" : "FAIL") << '\n';
        // If we pass, note the pass.
        if (result.pass) {
          ++packagePasses;
          ++subPackagePasses;
        }
        // If we fail, potentially print the diff.
        else {
          failed = true;
          if (!cfg.isQuiet() && !result.error)
            std::cout << '\n' << result.diff << '\n';
        }
      }
      std::cout << "  Subpackage passed " << subPackagePasses << " / " << subPackage.size()
                << '\n';
    }
    toolChainPasses += packagePasses;
    toolChainCount += packageCount;

    std::cout << " Package passed " << packagePasses<< " / " << packageCount << '\n';
  }
  
  std::cout << "Toolchain passed " << toolChainPasses << " / " << toolChainCount << "\n\n";
  return failed;
} 
}