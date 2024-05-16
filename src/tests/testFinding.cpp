#include "tests/testUtil.h"

#include <algorithm>

// Namespace that holds our actual exported functions.
namespace tester {

// check if test files exist at directory given by path
bool hasTestFiles(const fs::path& path) {
  for (const auto& file : fs::recursive_directory_iterator(path)) {
    // if (fs::is_regular_file(file.status()) && file.path().extension() == ".test")
    if (fs::is_regular_file(file.status()))
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

// Recursively find subpackages and fill them. Move ownership of locally created subpackages 
// into the parent Package. 
void findSubpackageRecursive(const fs::path &packPath, tester::Package &package, std::string parentKey) {
  try {
    
    // Handle nested subpackages
    for (const auto& subDir : fs::directory_iterator(packPath)) {
      if (fs::is_directory(subDir)) {
        // construct subpackage name recursively
        std::string subpackageKey = parentKey + "." + subDir.path().stem().string(); 
        
        if (hasTestFiles(subDir)) { // Ignores empty directories
          tester::SubPackage subpackage;
          fillSubpackage(subpackage, subDir.path());
          package[subpackageKey] = std::move(subpackage);
        }

        findSubpackageRecursive(subDir, package, subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void createPackageFromDirectory(const fs::path& packagePath, Package& package) {
    
    // Handle top level tests immediately within the Package. 
    tester::SubPackage sp;
    fillSubpackage(sp, packagePath);
    if (!sp.empty()) {
      package[packagePath.stem().string()] = std::move(sp);
    }

    // Fill nested Subpackages.
    std::string modulePrefix = packagePath.filename().string();
    findSubpackageRecursive(packagePath, package, modulePrefix);
}

void fillModule(fs::path testsPath, tester::Module &module) {
  
  // Iterate over each package. One package corresponds to one teams/individuals tests. 
  for (const auto& dir: fs::directory_iterator(testsPath)) {
    const fs::path &packagePath = dir.path();
    tester::Package package;
    
    createPackageFromDirectory(packagePath, package);
    module[packagePath.filename().string()] = std::move(package);
  }
}

} // End namespace tests
