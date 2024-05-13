#ifndef TESTER_TESTFINDING_H
#define TESTER_TESTFINDING_H

#include "tests/PathMatch.h"
#include "tests/TestResult.h"
#include "toolchain/ToolChain.h"

#include <filesystem>
#include <map>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

class TestFile {
public:
  TestFile(const fs::path& path) : testPath(path) {} // Copy reference to path 
  fs::path testPath;

  // implicit conversion to fs::path
  operator fs::path() const {
    return testPath;
  };
  
};

// Test file typedefs.
typedef std::vector<PathMatch> PathList;
typedef std::map<std::string, PathList> TestSet;
typedef std::map<std::string, TestSet> PackageSet;

// simplified test file typedefs
typedef std::vector<TestFile> SubPackage; // One Subpackage : Many Tests
typedef std::map<std::string, SubPackage> Package; // One Package : Many Subpackages
typedef std::map<std::string, Package> Module; // One Module : Many Packages

// Find tests to run.
void findTests(fs::path in, fs::path out, fs::path inStream, tester::PackageSet &tests);
void findTests(fs::path testsPath, tester::Module &module);

// Run a test.
TestResult runTest(const TestFile &testFile, const ToolChain &toolChain, bool quiet);

} // End namespace tester

#endif //TESTER_TESTFINDING_H
