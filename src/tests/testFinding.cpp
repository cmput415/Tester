#include "tests/testUtil.h"

#include <algorithm>

// Namespace that holds our actual exported functions.
namespace tester {

bool hasFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    if (fs::is_regular_file(entry.status())) {
      return true;
    }
  }
  return false;
}

tester::SubPackage fillSubpackage(fs::path subPackagePath) {
  tester::SubPackage testFiles; 
  for (const auto& testFile : fs::directory_iterator(subPackagePath)) {
    if (fs::is_regular_file(testFile)) {
      testFiles.push_back(TestFile(testFile)); 
    }
  }
  return testFiles;
}

void fillSubpackages(const fs::path &searchPath, tester::Package &package, const std::string &parentKey) {
  try {
    for (const auto& subDir : fs::directory_iterator(searchPath)) {
      if (fs::is_directory(subDir)) {
        std::string subpackageKey = parentKey + "." + subDir.path().stem().string(); 
        
        if (hasFiles(subDir)) {
          tester::SubPackage subpackage = fillSubpackage(subDir.path());
          package[subpackageKey] = std::move(subpackage);
        }

        fillSubpackages(subDir, package, subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void findTests(fs::path testsPath, tester::Module &module) {
  // Iterate over each package. One package corresponds to one teams/individuals tests. 
  for (const auto& dir: fs::directory_iterator(testsPath)) {
    const fs::path &packagePath = dir.path();
    const std::string &packageKeyPrefix = packagePath.filename();
    tester::Package package;
     
    fillSubpackages(packagePath, package, packageKeyPrefix);
    module[packageKeyPrefix] = package;
  }
}

} // End namespace tests
