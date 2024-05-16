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

  fs::path getTestPath() { return testPath; }
  fs::path getInsPath() { return insPath; }

  // Get read-only reference to the checklines vector
  const std::vector<std::string>& getCheckLines() const { return checkLines; }

private:

  // keywords
  const char *input_file_directive = "INPUT_FILE:";
  const char *input_directive = "INPUT:";
  const char *check_directive = "CHECK:";
  
  // Flags to indicate if each directive has been parsed 
  bool hasInput, hasCheck, hasInputFile;

  // Test file breaks some convention or was unable to parse directives. 
  bool badTest;
  std::string testErrorMessage;

  // Path for the .test (supplied in contructor) and .ins files (generated or supplied in INPUT_FILE). 
  fs::path testPath, insPath;
  
  // A vector containing contents of each CHECK directive in the .test file. 
  std::vector<std::string> checkLines;
 
  // implicit conversion to fs::path
  operator fs::path() const { return testPath; };

  // Parse a .test file to find directives and validate conventions
  void parse();

  // Parses the .test file for INPUT directives and fills
  // a temporary .ins file with the found contents.   
  void fillInputStreamFile();

  // Parses the .test file for CHECK directives and fills the
  // checkLines vector with each line found. 
  void fillCheckLines();

};

} // namespace tester

#endif //TESTER_TEST_FILE_H