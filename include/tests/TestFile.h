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
  
  // no default constructor
  TestFile() = delete;

  // construct Testfile from path to .test file. 
  TestFile(const fs::path& path) : testPath(path) {
    fillInputStreamFile();
    fillCheckLines();
  }

  // Delete the temporary .ins file created along with the class instance. 
  ~TestFile();

  // Does the test file contain non-empty INPUT & CHECK directives
  bool hasInput, hasCheck;

  // Path for the .test and .ins files. The former exists beforehand, the
  // later is created in fillInputStreamFile.
  fs::path testPath, insPath;
  
  // A vector containing contents of each CHECK directive in the .test file. 
  std::vector<std::string> checkLines;

  const char *input_directive = "INPUT:";
  const char *check_directive = "CHECK:";
  
  // implicit conversion to fs::path
  operator fs::path() const { return testPath; };

  // Parses the .test file for INPUT directives and fills
  // a temporary .ins file with the found contents.   
  void fillInputStreamFile();

  // Parses the .test file for CHECK directives and fills the
  // checkLines vector with each line found. 
  void fillCheckLines();

};

} // namespace tester

#endif //TESTER_TEST_FILE_H