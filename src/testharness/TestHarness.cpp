#include "testharness/TestHarness.h"

#include "tests/TestResult.h"
#include "tests/TestRunning.h"
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

void TestHarness::printTestResult(const TestFile *test, TestResult result) {
  std::cout << "    "
            << (result.pass ? (Colors::GREEN + "[PASS]" + Colors::RESET)
                            : (Colors::RED + "[FAIL]" + Colors::RESET))
            << " " << std::setw(40) << std::left << test->getTestPath().stem().string();
  if (cfg.isTimed()) {
    double time = test->getElapsedTime();

    if (time != 0) {
      std::cout << std::fixed << std::setw(10) << std::setprecision(6)
                << test->getElapsedTime() << "(s)";
    }
  }
  std::cout << "\n";
}

bool TestHarness::runTestsForToolChain(std::string exeName, std::string tcName) {
  bool failed = false;

  ToolChain toolChain = cfg.getToolChain(tcName); // Get the toolchain to use.
  const fs::path& exe = cfg.getExecutablePath(exeName); // Set the toolchain's exe to be tested.
  toolChain.setTestedExecutable(exe);

  if (cfg.hasRuntime(exeName)) // If we have a runtime, set that as well.
    toolChain.setTestedRuntime(cfg.getRuntimePath(exeName));
  else
    toolChain.setTestedRuntime("");

  std::cout << "\nTesting executable: " << exeName << " -> " << exe << '\n';
  std::cout << "With toolchain: " << tcName << " -> " << toolChain.getBriefDescription() << '\n';

  unsigned int toolChainCount = 0, toolChainPasses = 0; // Stat tracking for toolchain tests.

  // Iterate over each package.
  for (auto& [packageName, package] : testSet) {
    std::cout << "Entering package: " << packageName << '\n';
    unsigned int packageCount = 0, packagePasses = 0;

    // Iterate over each subpackage
    for (auto& [subPackageName, subPackage] : package) {
      std::cout << "  Entering subpackage: " << subPackageName << '\n';
      unsigned int subPackagePasses = 0, subPackageSize = subPackage.size();

      // Iterate over each test in the package
      for (size_t i = 0; i < subPackage.size(); ++i) {
        std::unique_ptr<TestFile>& test = subPackage[i];
        if (test->getParseError() == ParseError::NoError) {
        
          TestResult result = runTest(test.get(), toolChain, cfg);
          results.addResult(exeName, tcName, subPackageName, result);
          printTestResult(test.get(), result); 

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
              << "  Error: " << Colors::YELLOW << test->getParseErrorMsg() << Colors::RESET << "\n";
  }
  std::cout << "\n";

  return failed;
}

bool isTestFile(const fs::path& path) {
  return (fs::exists(path) 
      && !fs::is_directory(path)
      && path.extension() != ".ins"
      && path.extension() != ".out"
      && !(path.filename().string()[0] == '.') // don't consume hidden files
  );
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
  auto testfile = std::make_unique<TestFile>(file, testArtifactsPath);

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

  // If a debug path is supplied, override the testDirPath
  const std::optional<fs::path>& debugPath = cfg.getDebugPath();
  if (debugPath.has_value()) {
    setupDebugModule(testSet, *debugPath);
    return;
  }

  // Recursively find tests in the path containing testifles
  const fs::path& testDirPath = cfg.getTestDirPath();
  for (const auto& dir : fs::directory_iterator(testDirPath)) {

    if (dir.is_regular_file() && dir.path().filename().string()[0] == '.') {
      // skip hidden files like .gitkeep
      continue;
    } else if (!fs::is_directory(dir)) {
      // all other files should be directories
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
