#include "tests/TestFile.h"
#include "tests/TestParser.h"

namespace {

std::string stripFileExtension(const std::string& str) {
  size_t lastIdx = str.find_last_of(".");
  return str.substr(0, lastIdx);
}

} // anonymous namespace

namespace tester {

uint64_t TestFile::nextId = 0;

TestFile::TestFile(const fs::path& path) : id(++nextId), testPath(path) {

  // create a unique temporary file to use as the inputs stream path
  std::string fileInsPath = stripFileExtension(testPath.filename());
  insPath =
      fs::temp_directory_path() / (fileInsPath + std::to_string(id) + ".ins");
  outPath =
      fs::temp_directory_path() / (fileInsPath + std::to_string(id) + ".out");

  std::ofstream makeInsFile(insPath);
  std::ofstream makeOutFile(outPath);

  // closing creates the files
  makeInsFile.close();
  makeOutFile.close();
}

TestFile::~TestFile() {
  // clean up allocated resources on Testfile de-allocation
  if (usesInputStream && !usesInputFile) {
    fs::remove(insPath);
  }
  if (usesOutStream && !usesOutFile) {
    fs::remove(outPath);
  }
}

std::string TestFile::getErrorMessage() const {

  switch (getErrorState()) {
    case ParseError::NoError:
      return "No error";
      break;
    case ParseError::DirectiveConflict:
      return "Two or more testfile directives supplied that can not "
             "coexist in one file.";
      break;
    case ParseError::MaxInputBytesExceeded:
      return "Total bytes used for input stream exceeds maximum";
      break;
    case ParseError::MaxOutputBytesExceeded:
      return "Total bytes used for output stream exceeds maximum";
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
      return "No matching Parse State";
  }
}

} // namespace tester