#ifndef TESTER_TEST_FILE_H
#define TESTER_TEST_FILE_H

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
  TestFile() = delete;

  // construct Testfile from path to .test file.
  TestFile(const fs::path& path);
  ~TestFile();

  uint64_t id;

  // getters
  fs::path getTestPath() const { return testPath; }
  fs::path getInsPath() const { return insPath; }
  fs::path getOutPath() const { return outPath; }
  ParseError getParseError() const { return errorState; }
  std::string getParseErrorMsg() const;
  double getElapsedTime() const { return elapsedTime; }
  bool didError() const { return errorState != ParseError::NoError; }

  // setters
  void setTestPath(fs::path path) { testPath = path; }
  void setInsPath(fs::path path) { insPath = path; }
  void setOutPath(fs::path path) { outPath = path; }
  void getParseError(ParseError error) { errorState = error; }
  void setParseErrorMsg(std::string msg) { errorMsg = msg; }
  void setElapsedTime(double elapsed) { elapsedTime = elapsed; }

  // if test has any input and if test uses input file specifically
  bool usesInputStream{false}, usesInputFile{false}; 
  bool usesOutStream{false}, usesOutFile{false};

  friend class TestParser;

protected:
  static uint64_t nextId;

private:
  // Path for the test, ins and out files 
  fs::path testPath, insPath, outPath;

  // Test file breaks some convention or was unable to parse directives.
  ParseError errorState{ParseError::NoError};
  std::string errorMsg;

  // elapsed time for final toolchain step
  double elapsedTime{0};
};

} // namespace tester

#endif // TESTER_TEST_FILE_H