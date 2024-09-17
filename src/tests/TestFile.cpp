#include "tests/TestFile.h"
#include "tests/TestParser.h"

namespace {

std::string stripFileExtension(const std::string& str) {
  std::size_t lastIdx = str.find_last_of(".");
  return str.substr(0, lastIdx);
}

} // anonymous namespace

namespace tester {

uint64_t TestFile::nextId = 0;

TestFile::TestFile(const fs::path& path, const fs::path& tmpPath)
  : testPath(path) {

  setInsPath(tmpPath / std::to_string(nextId) / "test.ins");
  setOutPath(tmpPath / std::to_string(nextId) / "test.ins");
  
  std::cout << "Use tmp path: " << tmpPath << std::endl; 
  std::cout << "Out path: " << outPath << std::endl; 
  std::cout << "In path: " << insPath << std::endl; 

  nextId++;
}

TestFile::~TestFile() {
  if (fs::exists(insPath)) {
    fs::remove(insPath);
  }
  if (fs::exists(outPath)) {
    fs::remove(outPath);
  }
}

std::string TestFile::getParseErrorMsg() const {

  switch (getParseError()) {
    case ParseError::NoError:
      return "No error";
      break;
    case ParseError::DirectiveConflict:
      return "Two or more testfile directives supplied that can not "
             "coexist in one file.";
      break;
    case ParseError::FileError:
      return "A filepath provided in the testfile was unable to be "
             "located or opened.";
      break;
    case ParseError::RuntimeError:
      return "An unexpected runtime error occured while parsing the "
             "testifle.";
      break;
    default:
      return "No matching Parse Error";
  }
}

} // namespace tester