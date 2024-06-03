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

enum ParseError {
  NoError,
  DirectiveConflict,
  MaxInputBytesExceeded,
  MaxOutputBytesExceeded,
  FileError,
  RuntimeError
};

// forward declaration
class TestParser;

class TestFile {
public:
  
  // no default constructor
  TestFile() = delete;

  // construct Testfile from path to .test file. 
  TestFile(const fs::path& path);
  ~TestFile();

  uint64_t id;
   
  // getters
  fs::path getTestPath() const { return testPath; }
  fs::path getInsPath() const { return insPath; }
  fs::path getOutPath() const { return outPath; }
  ParseError getErrorState() const { return errorState; }
  std::string getErrorMessage() const;
  double getElapsedTime() const { return elapsedTime; }
  bool didError() const { return errorState != ParseError::NoError; }

  //  
  void setTestPath(fs::path path) { testPath = path; }
  void setInsPath(fs::path path) { insPath = path; }
  void setOutPath(fs::path path) { outPath = path; }
  void setErrorState(ParseError error) { errorState = error; }
  void setErrorMsg (std::string msg) { errorMsg = msg; } 
  void setElapsedTime(double elapsed) { elapsedTime = elapsed; }

  // if test has any input and if test uses input file specifically
  bool usesInputStream{false}, usesInputFile{false}, usesOutStream{false}, usesOutFile{false};

  // TODO: 
  friend class TestParser;

protected:
  static uint64_t nextId;

private:
   // Path for the .test (supplied in contructor) and .ins files (generated or supplied in INPUT_FILE). 
  fs::path testPath, insPath, outPath;
  
  // Test file breaks some convention or was unable to parse directives. 
  ParseError errorState{ParseError::NoError}; 
  std::string errorMsg;

  // elapsed time for final toolchain step
  double elapsedTime{0};
 
};

} // namespace tester

#endif //TESTER_TEST_FILE_H