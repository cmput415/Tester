// #include <map>
// #include <vector>
// #include <string>
// #include <memory>

#include "config/Config.h"
#include "TestFile.h"
#include "TestResult.h"
// #include "toolchain/ToolChain.h"
// #include "Colors.h"

namespace tester {

// typedef std::vector<std::unique_ptr<TestFile>> SubPackage; 
// typedef std::map<std::string, SubPackage> Package;
// typedef std::map<std::string, Package> TestModule;

// // test finding interface
// void fillModule(const Config &cfg, TestModule& module);

TestResult runTest(TestFile *test, const ToolChain &toolChain, const Config &cfg);

} // namespace tester

