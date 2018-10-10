#ifndef TESTER_TESTFINDING_H
#define TESTER_TESTFINDING_H

#include "tests/PathPair.h"
#include "tests/TestResult.h"
#include "toolchain/ToolChain.h"

#include <experimental/filesystem>
#include <map>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

// Test file typedefs.
typedef std::vector<PathPair> PathList;
typedef std::map<std::string, PathList> TestSet;
typedef std::map<std::string, TestSet> PackageSet;

// Find tests to run.
void findTests(fs::path in, fs::path out, tester::PackageSet &tests);

// Run a test.
TestResult runTest(const PathPair &tp, const ToolChain &toolChain, bool quiet);

} // End namespace tester

#endif //TESTER_TESTFINDING_H
