#ifndef TESTER_TEST_FILE_H
#define TESTER_TEST_FILE_H

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <optional>

namespace fs = std::filesystem;

namespace tester {

enum ParseError {
  NoError,
  DirectiveConflict,
  FileError,
  RuntimeError
};

// forward declaration
class TestParser;

class TestFile {
public:
  TestFile() = delete;
  // construct Testfile from path to .test file.
  TestFile(const fs::path& path, const fs::path& tmpPath);
  ~TestFile();

  uint64_t id;

  // Test path getters
  const fs::path& getTestPath() const { return testPath; }
  const fs::path& getOutPath() const { return outPath; }
  const fs::path& getInsPath() const { return insPath; }

  // Test state getters 
  ParseError getParseError() const { return errorState; }
  std::string getParseErrorMsg() const;
  double getElapsedTime() const { return elapsedTime; }
  bool didError() const { return errorState != ParseError::NoError; }

  // Test path getters
  void setInsPath(fs::path path) { insPath = path; }
  void setOutPath(fs::path path) { outPath = path; }

  // Test path setters
  void setParseError(ParseError error) { errorState = error; }
  void setParseErrorMsg(std::string msg) { errorMsg = msg; }
  void setElapsedTime(double elapsed) { elapsedTime = elapsed; }

  friend class TestParser;

protected:
  static uint64_t nextId;

private:
  // Path for the test, ins and out files 
  fs::path testPath;
  fs::path outPath;
  fs::path insPath;

  // Test file breaks some convention or was unable to parse directives.
  ParseError errorState{ParseError::NoError};
  std::string errorMsg;

  // elapsed time for final toolchain step
  double elapsedTime{0};
};

} // namespace tester

#endif // TESTER_TEST_FILE_H