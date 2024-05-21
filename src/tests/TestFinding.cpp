#include "tests/Util.h"

namespace tester {

bool hasTestFiles(const fs::path& path) {
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (fs::is_regular_file(entry.status()))
            return true;
    }
    return false;
}

bool isTestFile(const fs::path& path) {
  if (fs::exists(path) && path.extension() == ".test")
    return true;
  return false; 
}

void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath) {  

  for (const auto& file : fs::directory_iterator(subPackPath)) {
    if (fs::exists(file)) {
      
#if defined(DEBUG)
  std::cout << "Found Testfile: " << file << std::endl;
  std::cout << "Is testfile: " << isTestFile(file) << std::endl;  
#endif

      subPackage.push_back(std::make_unique<TestFile>(file));
    }
  }
}

void fillSubpackages(Package& package, const fs::path& packPath, const std::string& parentKey) {
    try {
        SubPackage topLevelSubPackage; // tests may be structed without any nested subpackages.
        for (const auto& file : fs::directory_iterator(packPath)) {

            if (isTestFile(file)) {
              topLevelSubPackage.push_back(std::make_unique<TestFile>(file));
            } 

            if (fs::is_directory(file)) {
                std::string subpackageKey = parentKey + "." + file.path().stem().string();
 
#if defined(DEBUG)
  std::cout << "Found subdirectory: " << subDir << std::endl;
#endif               
                if (hasTestFiles(file)) {
                    SubPackage subpackage;
                    fillSubpackage(subpackage, file.path());
                    package[subpackageKey] = std::move(subpackage);
                }

                fillSubpackages(package, file, subpackageKey);
            }
        }

        if (!topLevelSubPackage.empty()) {
          package[parentKey] = std::move(topLevelSubPackage);
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

// 
void fillModule(fs::path testsPath, TestModule& module) {

#if defined(DEBUG)
  std::cout << "Filling the test module at path: " << testsPath.string() << std::endl;
#endif

  for (const auto& dir : fs::directory_iterator(testsPath)) {
      
    const fs::path& packagePath = dir.path();
    const std::string& packageKeyPrefix = packagePath.filename().string();

    // initialize an empty package for this student / team 
    Package package;

    // recursively fill the package with subpackages 
    fillSubpackages(package, packagePath, "");

    // move the filled package into the test module.
    module[packageKeyPrefix] = std::move(package);
  }
}

}