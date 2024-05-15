#ifndef TESTER_TESTFINDING_H
#define TESTER_TESTFINDING_H

#include "tests/TestResult.h"
#include "tests/TestFile.h"
#include "../toolchain/ToolChain.h"

#include <filesystem>
#include <map>
#include <memory>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

// Simplified types which correspond to testing terminology
typedef std::vector<std::unique_ptr<TestFile>> SubPackage; 
typedef std::map<std::string, SubPackage> Package;
typedef std::map<std::string, Package> Module;

// Fill test module with Tests recursively.
void fillModule(fs::path testsPath, tester::Module& module);

// Run a test.
TestResult runTest(const std::unique_ptr<TestFile>& testFile, const ToolChain& toolChain, bool quiet);

} // End namespace tester

#endif //TESTER_TESTFINDING_H
