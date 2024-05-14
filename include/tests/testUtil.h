#ifndef TESTER_TESTFINDING_H
#define TESTER_TESTFINDING_H

#include "tests/PathMatch.h"
#include "tests/TestResult.h"
#include "tests/TestFile.h"
#include "../toolchain/ToolChain.h"

#include <filesystem>
#include <map>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

// simplified test file typedefs
typedef std::vector<TestFile> SubPackage; // One Subpackage : Many Tests
typedef std::map<std::string, SubPackage> Package; // One Package : Many Subpackages
typedef std::map<std::string, Package> Module; // One Module : Many Packages

// Find tests to run.
void findTests(fs::path testsPath, tester::Module &module);

// Run a test.
TestResult runTest(const TestFile &testFile, const ToolChain &toolChain, bool quiet);

} // End namespace tester

#endif //TESTER_TESTFINDING_H
