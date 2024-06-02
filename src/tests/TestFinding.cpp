#include "tests/Util.h"

namespace tester {

bool isTestFile(const fs::path& path) {
  return fs::exists(path) && !fs::is_directory(path) && path.extension() != ".ins" 
                          && path.extension() != ".out";
}

bool hasTestFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    if (isTestFile(entry)) {
      return true;
    }
  }
  return false;
}

void addTestFileToSubPackage(SubPackage& subPackage, const fs::path& file) {
  try {
    auto testfile = std::make_unique<TestFile>(file);
    subPackage.push_back(std::move(testfile));
  } catch (const std::exception& e) {
    std::cerr << "Exception creating TestFile: " << e.what() << std::endl;
  }
}

void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath) {
  for (const fs::path& file : fs::directory_iterator(subPackPath)) {
    if (isTestFile(file)) {
      addTestFileToSubPackage(subPackage, file);
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
        fillSubpackages(package, file.path(), subpackageKey);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void setupDebugModule(TestModule& module, const fs::path &debugPath) {
  Package debugPackage;
  SubPackage debugSubpackage;
  std::string prefix("debugSubPackage");

  if (fs::is_directory(debugPath)) {
    if (hasTestFiles(debugPath)) {
      fillSubpackage(debugSubpackage, debugPath);
      if (!debugSubpackage.empty()) {
        debugPackage[prefix] = std::move(debugSubpackage);
      }
      fillSubpackages(debugPackage, debugPath, prefix);
    }
  } else if (fs::exists(debugPath) && isTestFile(debugPath)) {
    addTestFileToSubPackage(debugSubpackage, debugPath);
    debugPackage[prefix] = std::move(debugSubpackage);
  } else {
    throw std::runtime_error("Bad debug path supplied.");
  }

  module["debugPkg"] = std::move(debugPackage);
}

void fillModule(const Config &cfg, TestModule& module) {
  const fs::path& debugPath = cfg.getDebugPath();
  if (!debugPath.empty()) {
    setupDebugModule(module, debugPath);
    return;
  }

  const fs::path& testDirPath = cfg.getTestDirPath();
  for (const auto& dir : fs::directory_iterator(testDirPath)) {
    if (!fs::is_directory(dir)) {
      throw std::runtime_error("All top-level files in module must be directories.");
    }

    const fs::path& packagePath = dir.path();
    const std::string& packageKeyPrefix = packagePath.filename().string();

    Package package;
    SubPackage moduleSubpackage;
    fillSubpackage(moduleSubpackage, packagePath);
    if (!moduleSubpackage.empty()) {
      package[packageKeyPrefix] = std::move(moduleSubpackage);
    }

    fillSubpackages(package, packagePath, packageKeyPrefix);
    module[packageKeyPrefix] = std::move(package);
  }
}

} // namespace tester