#ifndef TESTER_TEST_FILE_H
#define TESTER_TEST_FILE_H

#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

namespace tester {

class TestFile {
public:  

  TestFile() = delete;
  TestFile(const fs::path& path);  
  ~TestFile();
  
  // members
  fs::path testPath, inputStrPath;
  
  std::vector<std::string> checkLines;
  bool hasInput, hasCheck, hasInputFile;
  bool badTest;
  std::string testErrorMessage; 
  
  // Get read-only reference to the checklines vector
  const std::vector<std::string>& getCheckLines() const { return checkLines; }
 
};

} // namespace tester

#endif //TESTER_TEST_FILE_H