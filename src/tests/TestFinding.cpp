#include "tests/Util.h"

namespace tester {

bool isTestFile(const fs::path& path) {
  if (fs::exists(path) 
    && !fs::is_directory(path)
    && path.extension() != ".ins" 
    && path.extension() != ".out"
  ) {
    return true;
  }
  return false; 
}

bool hasTestFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    if (isTestFile(entry))
        return true;
  }
  return false;
}

void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath) {  

  for (const fs::path& file : fs::directory_iterator(subPackPath)) {
    if (fs::exists(file) && isTestFile(file)) {

      try {
        auto testfile = std::make_unique<TestFile>(file);
        // move ownership of the testfile to the subpackage
        subPackage.push_back(std::move(testfile));
      } catch (const std::exception& e) {
        std::cerr << "Exception creating TestFile: " << e.what() << std::endl;
      }   
    }
  }
}

void fillSubpackages(Package& package, const fs::path& packPath, const std::string& parentKey) {
  try {
    for (const auto& file : fs::directory_iterator(packPath)) {
      if (fs::is_directory(file)) {
        std::string subpackageKey = parentKey + "." + file.path().stem().string();

        if (hasTestFiles(file)) {
          SubPackage subpackage;
          fillSubpackage(subpackage, file.path());
          package[subpackageKey] = std::move(subpackage);
        }

        fillSubpackages(package, file, subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

// 
void fillModule(fs::path testsPath, TestModule& module) {

  for (const auto& dir : fs::directory_iterator(testsPath)) {

    if (!fs::is_directory(dir)) {
      throw std::runtime_error("All toplevel files in module must be directories.");
    }       

    const fs::path& packagePath = dir.path();
    const std::string& packageKeyPrefix = packagePath.filename().string();

    // initialize an empty package for this student / team 
    Package package;

    // initliaze some toplevel test files that have no subpackage in a default subpackage.
    SubPackage moduleSubpackage;
    fillSubpackage(moduleSubpackage, packagePath); 
    if (!moduleSubpackage.empty())
      package[packageKeyPrefix] = std::move(moduleSubpackage);

    // recursively fill the package with subpackages 
    fillSubpackages(package, packagePath, packageKeyPrefix);

    // move the filled package into the test module.
    module[packageKeyPrefix] = std::move(package);
  }
}

}