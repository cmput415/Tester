#include <map>
#include <vector>
#include <string>
#include <memory>

#include "TestFile.h"

namespace tester {

typedef std::vector<std::unique_ptr<TestFile>> SubPackage; 
typedef std::map<std::string, SubPackage> Package;
typedef std::map<std::string, Package> TestModule;

// test finding interface
void fillModule(fs::path testsPath, TestModule& module);

} // namespace tester

