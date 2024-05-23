#ifndef TESTER_TEST_FILE_H
#define TESTER_TEST_FILE_H

// #include "TestParser.h"
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

namespace tester {

enum ErrorState {
  NoError,
  DirectiveConflict,
  MaxInputStreamExceeded,
  FileError,
  RuntimeError
};

class TestFile {
public:
  
  // no default constructor
  TestFile() = delete;

  // construct Testfile from path to .test file. 
  TestFile(const fs::path& path);
  ~TestFile();

  uint64_t id;
  
  // TODO: the getters and setters for this class are shallow abstractions, 
  // consider changing the design. Until then, declare some members public...
  
  // getters
  fs::path getTestPath() { return testPath; }
  fs::path getInsPath() { return insPath; }
  fs::path getCheckFilePath() { return checkFilePath; }
  const std::vector<std::string>& getCheckLines() const { return checkLines; }
  ErrorState getErrorState() const { return errorState; }
  const std::string &getErrorMessage() const { return errorMsg; }
  
  // setters 
  void setTestPath(fs::path path) { testPath = path; }
  void setInsPath(fs::path path) { insPath = path; }
  void setCheckFilePath(fs::path path) { checkFilePath = path; }

  void pushCheckLine(std::string line) { checkLines.push_back(line); }
  void setErrorState(ErrorState error) { errorState = error; }
  void setErrorMsg (std::string msg) { errorMsg = msg; } 
  
  // if test has any input and if test uses input file specifically
  bool usesInputStream, usesInputFile;

protected:
  static uint64_t nextId;

private:
  // for UID generation
  
  // Test file breaks some convention or was unable to parse directives. 
  ErrorState errorState; 
  std::string errorMsg;

  // Path for the .test (supplied in contructor) and .ins files (generated or supplied in INPUT_FILE). 
  fs::path testPath, insPath, checkFilePath;
  
  // A vector containing contents of each CHECK directive in the .test file. 
  std::vector<std::string> checkLines; 
};

} // namespace tester

#endif //TESTER_TEST_FILE_H