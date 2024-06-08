#include "testharness/TestHarness.h"

#include "tests/TestResult.h"
#include "tests/Util.h"
#include "util.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <utility>

namespace tester {

// Builds TestSet during object creation.

bool TestHarness::runTests() {
  bool failed = false;
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto& tcPair : cfg.getToolChains()) {
      if (runTestsForToolChain(exePair.first, tcPair.first) == 1)
        failed = true;
    }
  }
  return failed;
}

std::string TestHarness::getTestInfo() const {
  std::ostringstream oss;
  oss << "Test Information:\n\n";
  for (const auto& [packageName, subPackages] : testSet) {
    oss << "Package: " << packageName << " (" << subPackages.size() << " subpackages)\n";
    for (const auto& [subPackageName, tests] : subPackages) {
      oss << " - Subpackage: " << subPackageName << " (" << tests.size() << " tests)\n";
    }
  }
  oss << "Total Packages: " << testSet.size() << "\n";
  return oss.str();
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

  // Iterate over each package.
  for (auto& [packageName, package] : testSet) {
    std::cout << "Entering package: " << packageName << '\n';
    unsigned int packageCount = 0, packagePasses = 0;

    // Iterate over each subpackage
    for (auto& [subPackageName, subPackage] : package) {
      std::cout << "  Entering subpackage: " << subPackageName << '\n';
      unsigned int subPackagePasses = 0, subPackageSize = subPackage.size();

      // Iterate over each test in the package
      for (const std::unique_ptr<TestFile>& test : subPackage) {
        if (test->getErrorState() == ParseError::NoError) {

          TestResult result = runTest(test.get(), toolChain, cfg);
          results.addResult(exeName, tcName, subPackageName, result);

          std::cout << "    "
                    << (result.pass ? (Colors::GREEN + "[PASS]" + Colors::RESET)
                                    : (Colors::RED + "[FAIL]" + Colors::RESET))
                    << " " << std::setw(40) << std::left << test->getTestPath().stem().string();

          if (cfg.isTimed()) {
            std::cout << std::fixed << std::setw(10) << std::setprecision(6)
                      << test->getElapsedTime() << "(s)\n";
          } else {
            std::cout << "\n";
          }

          if (result.pass) {
            ++packagePasses;
            ++subPackagePasses;
          } else {
            failed = true;
          }

        } else {
          std::cout << "    " << (Colors::YELLOW + "[INVALID]" + Colors::RESET) << " "
                    << test->getTestPath().stem().string() << '\n';
          --subPackageSize;
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
  std::cout << "Invalid " << invalidTests.size() << " / " << toolChainCount + invalidTests.size()
            << "\n";

  for (auto& test : invalidTests) {
    std::cout << "  Skipped: " << test->getTestPath().filename().stem() << std::endl
              << "  Error: " << Colors::YELLOW << test->getErrorMessage() << Colors::RESET << "\n";
  }
  std::cout << "\n";

  return failed;
}

bool isTestFile(const fs::path& path) {
  return fs::exists(path) && !fs::is_directory(path) && path.extension() != ".ins" &&
         path.extension() != ".out";
}

bool hasTestFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    if (isTestFile(entry)) {
      return true;
    }
  }
  return false;
}

void TestHarness::addTestFileToSubPackage(SubPackage& subPackage, const fs::path& file) {
  auto testfile = std::make_unique<TestFile>(file);

  TestParser parser(testfile.get());

  if (testfile->didError()) {
    invalidTests.push_back(std::move(testfile));
  } else {
    subPackage.push_back(std::move(testfile));
  }
}

void TestHarness::fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath) {
  for (const fs::path& file : fs::directory_iterator(subPackPath)) {
    if (isTestFile(file)) {
      addTestFileToSubPackage(subPackage, file);
    }
  }
}

void TestHarness::fillSubpackagesRecursive(Package& package, const fs::path& packPath,
                                           const std::string& parentKey) {
  try {
    for (const auto& file : fs::directory_iterator(packPath)) {
      if (fs::is_directory(file)) {
        std::string subpackageKey = parentKey + "." + file.path().stem().string();
        if (hasTestFiles(file)) {
          SubPackage subpackage;
          fillSubpackage(subpackage, file.path());
          package[subpackageKey] = std::move(subpackage);
        }
        fillSubpackagesRecursive(package, file.path(), subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void TestHarness::setupDebugModule(TestSet& testSet, const fs::path& debugPath) {

  Package debugPackage;
  SubPackage debugSubpackage;
  std::string prefix("debugSubPackage");

  if (fs::is_directory(debugPath)) {
    if (hasTestFiles(debugPath)) {
      fillSubpackage(debugSubpackage, debugPath);
      if (!debugSubpackage.empty()) {
        debugPackage[prefix] = std::move(debugSubpackage);
      }
      fillSubpackagesRecursive(debugPackage, debugPath, prefix);
    }
  } else if (fs::exists(debugPath) && isTestFile(debugPath)) {
    addTestFileToSubPackage(debugSubpackage, debugPath);
    debugPackage[prefix] = std::move(debugSubpackage);
  } else {
    throw std::runtime_error("Bad debug path supplied.");
  }

  testSet["debugPkg"] = std::move(debugPackage);
}

void TestHarness::findTests() {

  const fs::path& debugPath = cfg.getDebugPath();
  const fs::path& testDirPath = cfg.getTestDirPath();

  if (!debugPath.empty()) {
    setupDebugModule(testSet, debugPath);
    return;
  }

  for (const auto& dir : fs::directory_iterator(testDirPath)) {
    if (!fs::is_directory(dir)) {
      throw std::runtime_error("All top-level files in module must be directories.");
    }

    const fs::path& packagePath = dir.path();
    const std::string& packageKeyPrefix = packagePath.filename().string();

    Package package;
    SubPackage moduleSubpackage;

    fillSubpackage(moduleSubpackage, packagePath);
    if (!moduleSubpackage.empty()) {
      package[packageKeyPrefix] = std::move(moduleSubpackage);
    }

    fillSubpackagesRecursive(package, packagePath, packageKeyPrefix);
    testSet[packageKeyPrefix] = std::move(package);
  }
}

} // End namespace tester
