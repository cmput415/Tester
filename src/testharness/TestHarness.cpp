#include "testharness/TestHarness.h"

#include "tests/TestResult.h"
#include "tests/TestRunning.h"
#include "util.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
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
  std::vector<std::thread> threadPool;
  std::vector<std::reference_wrapper<TestPair>> flattenedList;
  std::vector<std::string> exeList;
  std::vector<std::string> tcList;

  static size_t currentIndex = 0;
  std::mutex currentIndexLock;

  // Initialize the threads
  // Iterate over executables.
  for (auto exePair : cfg.getExecutables()) {
    // Iterate over toolchains.
    for (auto& tcPair : cfg.getToolChains()) {
      // iterate over packages
      for (auto& package : testSet) { // TestSet.second -> Package
        // iterate over subpackages
        for (auto& subpackage : package.second) { // Package.second -> SubPackage
          // populate the flattened test vector
          for (auto& test : subpackage.second) {
            flattenedList.push_back(std::ref(test));
            exeList.push_back(exePair.first);
            tcList.push_back(tcPair.first);
          }
        }
      }
    }
  }

  // Let the load balancing be the responsability of the threads, instead of the supervisor
  for (int64_t i = 0; i < cfg.getNumThreads(); i++) {
    std::thread t(&TestHarness::threadRunTestBatch, this,
                  std::ref(tcList),
                  std::ref(exeList),
                  std::ref(flattenedList),
                  std::ref(currentIndex),
                  std::ref(currentIndexLock));
    threadPool.push_back(std::move(t));
  }

  // KILL ALL THE CHILDREN
  for (auto& thread : threadPool) thread.join();
}

void TestHarness::threadRunTestBatch(std::reference_wrapper<std::vector<std::string>> toolchains,
                        std::reference_wrapper<std::vector<std::string>> executables,
                        std::reference_wrapper<std::vector<std::reference_wrapper<TestPair>>> tests,
                        std::reference_wrapper<size_t> currentIndex,
                        std::reference_wrapper<std::mutex> currentIndexLock)
{
  // Loop while we have not exhausted the available tests
  while (currentIndex < tests.get().size()) {
    size_t tmpIndex;

    {
      // Lock the index
      std::lock_guard<std::mutex> lock(currentIndexLock);

      // store the current index
      tmpIndex = currentIndex;
      // increment for the next lad
      currentIndex += cfg.getBatchSize();
    }
    size_t endIndex = ((tmpIndex + cfg.getBatchSize()) >= tests.get().size()) ?  tests.get().size() : tmpIndex + cfg.getBatchSize();

    threadRunTestsForToolChain(std::ref(toolchains), std::ref(executables), std::ref(tests), tmpIndex, endIndex);
  }
}

void TestHarness::threadRunTestsForToolChain(std::reference_wrapper<std::vector<std::string>> tcNames,
                                             std::reference_wrapper<std::vector<std::string>> exeNames,
                                             std::reference_wrapper<std::vector<std::reference_wrapper<TestPair>>> tests,
                                             size_t begin, size_t end)
{
  for (size_t i = begin; i < end; i++) {
    ToolChain toolChain = cfg.getToolChain(tcNames.get().at(i)); // Get the toolchain to use.
    const fs::path& exe = cfg.getExecutablePath(exeNames.get().at(i)); // Set the toolchain's exe to be tested.
    toolChain.setTestedExecutable(exe);

    // set the runtime
    if (cfg.hasRuntime(exeNames.get().at(i))) // If we have a runtime, set that as well.
      toolChain.setTestedRuntime(cfg.getRuntimePath(exeNames.get().at(i)));
    else
      toolChain.setTestedRuntime("");

    std::unique_ptr<TestFile>& test = tests.get().at(i).get().first;
    if (test->getParseError() == ParseError::NoError) {

      TestResult result = runTest(test.get(), toolChain, cfg);
      // keep the result with the test for pretty printing
      std::optional<TestResult> res_clone = std::make_optional(result.clone());
      tests.get().at(i).get().second.swap(res_clone);
    }
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
