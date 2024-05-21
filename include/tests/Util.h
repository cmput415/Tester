#include <map>
#include <vector>
#include <string>
#include <memory>

#include "TestFile.h"
#include "TestResult.h"
#include "toolchain/ToolChain.h"

namespace tester {

typedef std::vector<std::unique_ptr<TestFile>> SubPackage; 
typedef std::map<std::string, SubPackage> Package;
typedef std::map<std::string, Package> TestModule;

// test finding interface
void fillModule(fs::path testsPath, TestModule& module);

TestResult runTest(const std::unique_ptr<TestFile> &test, const ToolChain &toolChain, bool quiet);

} // namespace tester

