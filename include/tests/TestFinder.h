#include "Util.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace tester {

class TestFinder {
public:
  void findTests();

private:
  TestModule module;
  fs::path basePath;

  bool hasTestFiles(const fs::path& path);
  void fillSubpackage(SubPackage& subPackage, const fs::path& subPackPath);
  void findSubpackageRecursive(const fs::path &packPath, tester::Package &package, std::string parentKey);
  void createPackageFromDirectory(const fs::path& packagePath, Package& package);
  void fillModule(fs::path testsPath, tester::TestModule &module);
};

} //namespace tester