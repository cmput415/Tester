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

// Test file typedefs.
typedef std::vector<PathMatch> PathList;
typedef std::map<std::string, PathList> TestSet;
typedef std::map<std::string, TestSet> PackageSet;

// simplified test file typedefs
typedef std::map<fs::path, std::vector<PathMatch>> SubPackage;
typedef std::map<fs::path, std::vector<SubPackage>> Package;
typedef std::map<std::string, Package> Module;

// Find tests to run.
void findTests(fs::path in, fs::path out, fs::path inStream, tester::PackageSet &tests);
void findTests(fs::path testsPath, tester::Module &module);

// Run a test.
TestResult runTest(const PathMatch &pm, const ToolChain &toolChain, bool quiet);

} // End namespace tester

#endif //TESTER_TESTFINDING_H
