#include "tests/testUtil.h"

#include <algorithm>

// Namespace that holds our actual exported functions.
namespace tester {

// check if test files exist at directory given by path
bool hasTestFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    // TODO: assert file extension equals .test
    if (fs::is_regular_file(entry.status()))
      return true;
  }
  return false;
}

// fill the vector of tests with TestFile objects in current directory path 
void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath) {
  for (const auto& file : fs::directory_iterator(subPackPath)) {
    if (fs::is_regular_file(file)) {
      subPackage.push_back(std::make_unique<TestFile>(file));
    }
  }
}

// recursively find subpackages and fill them. Move ownership of locally created subpackages 
// into the parent Package. 
void fillSubpackages(const fs::path &packPath, tester::Package &package, const std::string &parentKey) {
  try {
    for (const auto& subDir : fs::directory_iterator(packPath)) {
      if (fs::is_directory(subDir)) {
        std::string subpackageKey = parentKey + "." + subDir.path().stem().string(); 
        
        if (hasTestFiles(subDir)) {
          tester::SubPackage subpackage;
          fillSubpackage(subpackage, subDir.path());
          package[subpackageKey] = std::move(subpackage);
        }

        fillSubpackages(subDir, package, subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void fillModule(fs::path testsPath, tester::Module &module) {
  // Iterate over each package. One package corresponds to one teams/individuals tests. 
  for (const auto& dir: fs::directory_iterator(testsPath)) {
    const fs::path &packagePath = dir.path();
    const std::string &packageKeyPrefix = packagePath.filename();
    tester::Package package;
     
    fillSubpackages(packagePath, package, packageKeyPrefix);
    module[packageKeyPrefix] = std::move(package);
  }
}

} // End namespace tests
