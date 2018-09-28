#include "testharness/TestHarness.h"

#include "tests/TestResult.h"

#include "util.h"

#include <sstream>
#include <utility>
#include <iostream>
#include <experimental/filesystem>

namespace tester {

// Builds TestSet during object creation.
TestHarness::TestHarness(const JSON &json, bool quiet) : quiet(quiet) {
  // Make sure we have an executable to test then set it. Need to explicitly tell json what type
  // we're pulling out here because it doesn't like loading into an fs::path.
  ensureContains(json, "testedExecutablePaths");
  const JSON &tepJson = json["testedExecutablePaths"];
  if (!tepJson.is_object())
    throw std::runtime_error("Tested executable paths was not an object.");


  for (auto it = tepJson.begin(); it != tepJson.end(); ++it) {
    std::string path = it.value();
    executables.emplace(it.key(), path);
  }

  // Make sure toolchains are provided then build the set of toolchains.
  ensureContains(json, "toolchains");
  const JSON &tcJson = json["toolchains"];
  if (!tcJson.is_object())
    throw std::runtime_error("Toolchains is not an object.");

  for (auto it = tcJson.begin(); it != tcJson.end(); ++it) {
    toolchains.emplace(it.key(), it.value());
  }

  // Make sure an in and out dir were provided.
  ensureContains(json, "inDir");
  ensureContains(json, "outDir");

  // Get the in and out paths.
  std::string inDirStr = json["inDir"];
  std::string outDirStr = json["outDir"];
  fs::path inDir(inDirStr);
  fs::path outDir(outDirStr);

  // Ensure the paths exist.
  if (!fs::exists(inDir) || !fs::is_directory(inDir))
    throw std::runtime_error("Input file directory did not exist: " + inDirStr);
  if (!fs::exists(outDir) || !fs::is_directory(outDir))
    throw std::runtime_error("Output file directory did not exist: " + outDirStr);

  // Build the test set.
  findTests(inDir, outDir, tests);
}

void TestHarness::runTests() {
  // Iterate over executables.
  for (auto exePair : executables) {
    // Iterate over toolchains.
    for (auto &tcPair : toolchains) {
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
  for (const auto &exePair : executables) {
    // Write out this executables header.
    allInfo << exePair.first << ":\n";

    // The streams for package info. Self is for an exectuable's own test package, other for the
    // others.
    std::stringstream selfPackageInfo;
    std::stringstream otherPackageInfo;

    // We leave the above streams blank because their might not be tests in that category. These
    // bools track if we've written to them. If they're false and you're about to write to it then
    // you should add an appropriate header.
    bool toSelf = false, toOther = false;

    // Iterate over the test packages.
    for (const auto &testPair : tests) {
      // Success count.
      unsigned int passes = 0, count = 0;

      // Iterate over toolchains.
      for (const auto &tcPair : toolchains) {
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
  ToolChain &toolChain = toolchains.at(tcName);

  // Set the toolchain's exe to be tested.
  const fs::path &exe = executables.at(exeName);
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
        else if (!quiet && !result.error)
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
