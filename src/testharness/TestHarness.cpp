#include "testharness/TestHarness.h"

#include "tests/TestResult.h"

#include "util.h"

#include <sstream>
#include <utility>
#include <iostream>
#include <experimental/filesystem>

namespace tester {

// Builds TestSet during object creation.
TestHarness::TestHarness(const Config &cfg) : cfg(cfg), tests(), results() {
  // Build the test set.
  findTests(cfg.getInDirPath(), cfg.getOutDirPath(), tests);
}

void TestHarness::runTests() {
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto &tcPair : cfg.getToolChains()) {
      runTestsForToolChain(exePair.first, tcPair.first);
    }
  }
}

std::string TestHarness::getTestInfo() const {
  std::string rv = "Tests:\n";
  for (auto &tlEntry : tests) {
    rv += "  " + tlEntry.first + ": " + std::to_string(tlEntry.second.size()) + '\n';
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
    for (const auto &testPair : tests) {
      // Success count.
      unsigned int passes = 0, count = 0;

      // Iterate over toolchains.
      for (const auto &tcPair : cfg.getToolChains()) {
        // Get the list of results for this exe, toolchain, and package.
        const ResultList &packageResults =
          results.getResults(exePair.first, tcPair.first).at(testPair.first);

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
      if (exePair.first == testPair.first) {
        // We should only ever write to the self stream once, so toSelf is more of a trap than a
        // flag. We include the header in the one write as well.
        assert(!toSelf && "Already written to self");
        toSelf = true;
        selfPackageInfo << "  Self tests:\n    " << testPair.first << ": " << passes << " / " << count
                        << " -> " << (passes == count ? "PASS" : "FAIL") << '\n';
      }
      else {
        // Add other header.
        if (!toOther) {
          otherPackageInfo << "  Other tests:\n";
          toOther = true;
        }

        // Write info.
        otherPackageInfo << "    " << testPair.first << ": " << passes << " / " << count << '\n';
      }
    }

    allInfo << selfPackageInfo.str() << otherPackageInfo.str();
  }

  return allInfo.str();
}

void TestHarness::runTestsForToolChain(std::string exeName, std::string tcName) {
  // Get the toolchain to use.
  ToolChain toolChain = cfg.getToolChain(tcName);

  // Set the toolchain's exe to be tested.
  const fs::path &exe = cfg.getExecutablePath(exeName);
  std::cout << "\nTesting executable: " << exeName << " -> " << exe << '\n';
  toolChain.setTestedExecutable(exe);

  // Say which toolchain.
  std::cout << "With toolchain: " << tcName << " -> " <<  toolChain.getBriefDescription() << '\n';

  // Stat tracking for toolchain tests.
  unsigned int toolChainCount = 0, toolChainPasses = 0;

  // Iterate the packages.
  for (const auto &testPackage : tests) {
    // Print the package name.
    std::cout << "Entering package: " << testPackage.first << '\n';

    // Stat tracking for package tests.
    unsigned int packageCount = 0, packagePasses = 0;

    for (const auto &testSet : testPackage.second) {
      std::cout << "  Entering subpackage: " << testSet.first << '\n';

      // Track how many tests we run.
      packageCount += testSet.second.size();

      // Count how many passes we get.
      unsigned int subPackagePasses = 0;

      // Iterate over the tests.
      for (const PathPair &tp : testSet.second) {
        // Run the test and save the result.
        TestResult result = runTest(tp, toolChain);
        results.addResult(exeName, tcName, testPackage.first, result);

        // Log the pass/fail.
        std::cout << "    " << tp.in.stem().string() << ": "
                  << (result.pass ? "PASS" : "FAIL") << '\n';

        // If we pass, note the pass.
        if (result.pass) {
          ++packagePasses;
          ++subPackagePasses;
        }
        // If we fail, potentially print the diff.
        else if (!cfg.isQuiet() && !result.error)
          std::cout << '\n' << result.diff << '\n';
      }

      std::cout << "  Subpackage passed " << subPackagePasses << " / " << testSet.second.size()
                << '\n';
    }

    // Update the toolchain stats from the package stats.
    toolChainPasses += packagePasses;
    toolChainCount += packageCount;

    std::cout << " Package passed " << packagePasses<< " / " << packageCount << '\n';
  }

  std::cout << "Toolchain passed " << toolChainPasses << " / " << toolChainCount << "\n\n";
}

} // End namespace tester
