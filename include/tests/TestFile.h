#ifndef TESTER_TEST_FILE_H
#define TESTER_TEST_FILE_H

#include <filesystem>

namespace fs = std::filesystem;

namespace tester{

class TestFile {
public:
  TestFile() = default;
  TestFile(const fs::path& path) : testPath(path) {} // Copy reference to path 
  fs::path testPath;
  std::string stdin, stdout;
  
  // implicit conversion to fs::path
  operator fs::path() const {
    return testPath;
  };
  
};

} // namespace tester

#endif //TESTER_TEST_FILE_H