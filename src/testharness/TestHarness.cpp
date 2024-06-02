#include "testharness/TestHarness.h"

#include "tests/TestResult.h"
#include "tests/Util.h"
#include "util.h"

#include <sstream>
#include <utility>
#include <iostream>
#include <filesystem>

namespace tester {

// Builds TestSet during object creation.
TestHarness::TestHarness(const Config &cfg) : cfg(cfg), results(), module() {
  // Build the test set.
  fillModule(cfg, module);  
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
  for (auto &tlEntry : module) {
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
    for (const auto& [packageName, package] : module) {
      // Success count.
      unsigned int passes = 0, count = 0;

      // Iterate over toolchains.
      for (const auto &tcPair : cfg.getToolChains()) {
        // Get the list of results for this exe, toolchain, and package.
        const ResultList &packageResults =
          results.getResults(exePair.first, tcPair.first).at(packageName);

        // Iterate over the results.
        for (const auto &result : packageResults) {
          if (result.pass) {
            ++passes;
          }
        }

        // Save the size.
        count += packageResults.size();
      }

      // If this executable is linked with this package then it's the executable maker's set of
      // tests. We handle this slightly differently.
      if (exePair.first == packageName) {
        // We should only ever write to the self stream once, so toSelf is more of a trap than a
        // flag. We include the header in the one write as well.
        assert(!toSelf && "Already written to self");
        toSelf = true;
        selfPackageInfo << "  Self tests:\n    " << packageName << ": " << passes << " / " << count
                        << " -> " << (passes == count ? "PASS" : "FAIL") << '\n';
      }
      else {
        // Add other header.
        if (!toOther) {
          otherPackageInfo << "  Other tests:\n";
          toOther = true;
        }

        // Write info.
        otherPackageInfo << "    " << packageName << ": " << passes << " / " << count << '\n';
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
    const fs::path& exe = cfg.getExecutablePath(exeName);
    std::cout << "\nTesting executable: " << exeName << " -> " << exe << '\n';
    toolChain.setTestedExecutable(exe);

    // If we have a runtime, set that as well.
    if (cfg.hasRuntime(exeName))
        toolChain.setTestedRuntime(cfg.getRuntimePath(exeName));
    else
        toolChain.setTestedRuntime("");

    // Say which toolchain.
    std::cout << "With toolchain: " << tcName << " -> " << toolChain.getBriefDescription() << '\n';

    // Stat tracking for toolchain tests.
    unsigned int toolChainCount = 0, toolChainPasses = 0;

    // Track invalid tests
    std::vector<TestFile*> invalidTests;

    // Iterate over each package.
    for (auto& [packageName, package] : module) {
        std::cout << "Entering package: " << packageName << '\n';
        unsigned int packageCount = 0, packagePasses = 0;

        // Iterate over each subpackage
        for (auto& [subPackageName, subPackage] : package) {
            std::cout << "  Entering subpackage: " << subPackageName << '\n';
            unsigned int subPackagePasses = 0, subPackageSize = subPackage.size();

            // Iterate over each test in the package
            for (std::unique_ptr<TestFile>& test : subPackage) {
                if (test->getErrorState() == ParseError::NoError) {
                    TestResult result = runTest(test.get(), toolChain, cfg);
                    results.addResult(exeName, tcName, subPackageName, result);

                    std::cout << "    " << (result.pass ? (Colors::GREEN + "[PASS]" + Colors::RESET) : (Colors::RED + "[FAIL]" + Colors::RESET))
                              << " " << test->getTestPath().stem().string() << '\n';

                    if (result.pass) {
                        ++packagePasses;
                        ++subPackagePasses;
                    } else {
                        failed = true;
                        if (!cfg.isQuiet() && !result.error)
                            std::cout << '\n' << result.diff << '\n';
                    }
                } else {
                    std::cout << "    " << (Colors::YELLOW + "[INVALID]" + Colors::RESET)
                              << " " << test->getTestPath().stem().string() << '\n';
                    --subPackageSize;
                    invalidTests.push_back(test.get());
                }
            }
            std::cout << "  Subpackage passed " << subPackagePasses << " / " << subPackageSize << '\n';

            // Track how many tests we run.
            packageCount += subPackageSize;
        }

        // Update the toolchain stats from the package stats.
        toolChainPasses += packagePasses;
        toolChainCount += packageCount;

        std::cout << " Package passed " << packagePasses << " / " << packageCount << '\n';
    }

    std::cout << "Toolchain passed " << toolChainPasses << " / " << toolChainCount << "\n\n";
    std::cout << "Invalid " << invalidTests.size() << " / "
              << toolChainCount + invalidTests.size() << "\n";

    for (const TestFile* test : invalidTests) {
        std::cout << "  Skipped: " << test->getTestPath().filename().stem() << std::endl
                  << "  Error: " << Colors::YELLOW << test->getErrorMessage() << Colors::RESET << "\n";
    }
    std::cout << "\n";

    return failed;
}


} // End namespace tester
