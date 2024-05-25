#include "tests/TestFile.h"
#include "tests/TestParser.h"

namespace {

std::string stripFileExtension(const std::string &str) {
  size_t lastIdx = str.find_last_of(".");
  return str.substr(0, lastIdx);
}


} // anonymous namespace

namespace tester {

uint64_t TestFile::nextId = 0;

TestFile::TestFile(const fs::path& path) 
  :  id(++nextId), usesInputStream(false), testPath(path), 
    usesInputFile(false), errorState(ErrorState::NoError)
  {

  // create a unique temporary file to use as the inputs stream path 
  std::string fileInsPath = stripFileExtension(testPath.filename()); 
  insPath = fs::temp_directory_path() / (fileInsPath + std::to_string(id) + ".ins");
  outPath = fs::temp_directory_path() / (fileInsPath + std::to_string(id) + ".out");

  std::ofstream makeInsFile(insPath);
  std::ofstream makeOutFile(outPath);
  
  makeInsFile.close();
  makeOutFile.close(); 
  
  // invoke the parser
  auto parser = std::make_unique<TestParser>(*this);
}

TestFile::~TestFile() {
  if (usesInputStream && !usesInputFile) {
    fs::remove(insPath);
  }
}

} // namespace tester