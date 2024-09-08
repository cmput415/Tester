#include "testharness/TestHarness.h"

#include "tests/TestResult.h"
#include "tests/TestRunning.h"
#include "util.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <thread>
#include <utility>

namespace tester {

void swap(TestResult& first, TestResult& second) {
  std::swap(first, second);
}

// Builds TestSet during object creation.
bool TestHarness::runTests() {
  // initialize the threads
  std::thread t(&TestHarness::spawnThreads, this);

  bool failed = false;
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto& tcPair : cfg.getToolChains()) {
      if (aggregateTestResultsForToolChain(tcPair.first, exePair.first) == 1)
        failed = true;
    }
  }

  // join the control thread
  t.join();
  return failed;
}

void TestHarness::spawnThreads() {
  int16_t numThreads = 0;
  std::vector<std::thread> threadPool;

  // Initialize the threads
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto& tcPair : cfg.getToolChains()) {
      // iterate over packages
      for (auto& package : testSet) { // TestSet.second -> Package
        // iterate over subpackages
        for (auto& subpackage : package.second) { // Package.second -> SubPackage
          // If we have already spawned the maximum number of threads,
          //  wait for the first one to finish before spawning another.
          if (numThreads >= cfg.getNumThreads()) {
            threadPool.back().join();
            threadPool.pop_back();
            numThreads--;
          }

          // spawn a new thread executing its tests
          std::thread t(&TestHarness::threadRunTestsForToolChain, this, tcPair.first, exePair.first, std::ref(subpackage.second));
          threadPool.push_back(std::move(t));
          numThreads++;
        }
      }
    }
  }

  // Join any stragglers
  assert(threadPool.size() <= (size_t) cfg.getNumThreads());
  for (size_t i = 0; i < threadPool.size(); i++) {
    threadPool[i].join();
  }
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

bool TestHarness::aggregateTestResultsForToolChain(std::string tcName, std::string exeName) {
  bool failed = false;

  ToolChain toolChain = cfg.getToolChain(tcName); // Get the toolchain to use.
  const fs::path& exe = cfg.getExecutablePath(exeName); // Set the toolchain's exe to be tested.
  toolChain.setTestedExecutable(exe);

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
        TestPair& pair = subPackage[i];
        std::unique_ptr<TestFile>& test = pair.first;
        if (test->getParseError() == ParseError::NoError) {

          // Poll while we wait for the result
          // TODO this could probably be replaced with some sort of interrupt,
          //  (and probably should be), but better this than no threads
          while (!pair.second.has_value())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

          TestResult result = pair.second.value();

          // keep the result with the test for pretty printing
          std::optional<TestResult> res_clone = std::make_optional(result.clone());
          subPackage[i].second.swap(res_clone);

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
    std::cout << "  Skipped: " << test.first->getTestPath().filename().stem() << std::endl
              << "  Error: " << Colors::YELLOW << test.first->getParseErrorMsg() << Colors::RESET << "\n";
  }
  std::cout << "\n";

  std::cout << Colors::GREEN << "Completed Tests" << Colors::RESET << std::endl;
  std::cout << "Hold on while we clean up any remaining threads, this might take a moment" << std::endl;

  return failed;
}

void TestHarness::threadRunTestsForToolChain(std::string tcName, std::string exeName, std::reference_wrapper<SubPackage> subPackage) {
  std::cout << "Thread " << std::this_thread::get_id() << ": executing subpackage" << std::endl;
  ToolChain toolChain = cfg.getToolChain(tcName); // Get the toolchain to use.
  const fs::path& exe = cfg.getExecutablePath(exeName); // Set the toolchain's exe to be tested.
  toolChain.setTestedExecutable(exe);

  // set the runtime
  if (cfg.hasRuntime(exeName)) // If we have a runtime, set that as well.
    toolChain.setTestedRuntime(cfg.getRuntimePath(exeName));
  else
    toolChain.setTestedRuntime("");

  for (size_t i = 0; i < subPackage.get().size(); ++i) {
    std::unique_ptr<TestFile>& test = subPackage.get().at(i).first;
    if (test->getParseError() == ParseError::NoError) {

      TestResult result = runTest(test.get(), toolChain, cfg);
      // keep the result with the test for pretty printing
      std::optional<TestResult> res_clone = std::make_optional(result.clone());
      subPackage.get().at(i).second.swap(res_clone);
    }
  }
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
  auto testfile = std::make_unique<TestFile>(file);

  TestParser parser(testfile.get());

  std::optional<TestResult> no_result = std::nullopt;
  if (testfile->didError()) {
    invalidTests.push_back({std::move(testfile), no_result});
  }else {
    subPackage.push_back({std::move(testfile), no_result});
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
