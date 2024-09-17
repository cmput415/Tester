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

  fs::path testDir = tmpPath / std::to_string(nextId);
  setInsPath(testDir / "test.ins");
  setOutPath(testDir / "test.out");

  try {
    // Create tmp directory if it doesn't exist
    std::cout << "Attempting to create directory: " << testDir << std::endl;
    if (!fs::exists(testDir)) {
      if (!fs::create_directories(testDir)) {
        throw std::runtime_error("Failed to create directory: " + testDir.string());
      }
    }
    // Create the temporary input and ouput files
    std::ofstream createInsFile(insPath);
    std::ofstream createOutFile(outPath);
    if (!createInsFile) {
      throw std::runtime_error("Failed to create input file: " + insPath.string());
    }
    if (!createOutFile) {
      throw std::runtime_error("Failed to create output file: " + outPath.string());
    }
    createInsFile.close();
    createOutFile.close();

  } catch (const fs::filesystem_error& e) {
    throw std::runtime_error("Filesystem error: " + std::string(e.what()));
  } catch (const std::exception& e) {
    throw std::runtime_error("Error in TestFile constructor: " + std::string(e.what()));
  }
  nextId++;
}

TestFile::~TestFile() {
  if (fs::exists(insPath)) {
    // Remove temporary input stream file 
    fs::remove(insPath);
  }
  if (fs::exists(outPath)) {
    // Remove the tenmporary testfile directory and the expected out
    fs::path testfileDir = outPath.parent_path(); 
    fs::remove(outPath);
    fs::remove(testfileDir);
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