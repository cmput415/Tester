#include "tests/TestFile.h"

namespace tester {

TestFile::~TestFile() {

  std::error_code ec;
  fs::remove(insPath, ec);
  if (ec) {
    // TODO: whether or not this is right way to handle errors.
    std::cerr << "ERROR: Unable to delete temporary input stream file." << std::endl;
  }
}

void TestFile::fillInputStreamFile() {
 
  insPath = fs::temp_directory_path() / testPath.filename().replace_extension(".ins");
  std::ifstream testFile(testPath);
  std::ofstream insFile(insPath);
  
  // TODO: how to handle errors properly 
  if (!testFile.is_open()) {
    std::cerr << "error: failed to open test file: " << testPath << std::endl; 
    return;
  } else if (!insFile.is_open()) {
    std::cerr << "error: failed to open inStream file: " << insPath << std::endl; 
    return; 
  }

  // TODO: ensure that NEWLINES are not ignored. Should an empty INPUT: generate a string
  // with only a newline character?
  std::string line;
  while (std::getline(testFile, line)) {
    size_t findIdx = line.find(input_directive);
    if (findIdx != std::string::npos) {
      // write the conetnts following the INPUT directive into the input stream file.
      insFile << line.substr(findIdx + strlen(input_directive)) << std::endl;
      hasInput = true; 
    }
  }
 
  //DEBUG: print the contents of insFile
  std::cout << "DEBUG" << std::endl;
  std::ifstream dumpInsFile(insPath);
  std::string dumpLine;
  while (std::getline(dumpInsFile, dumpLine)) {
    std::cout << dumpLine << std::endl;
  } 
  std::cout << "END_DEBUG" << std::endl;

  // Close files
  insFile.close();
  testFile.close();  
}

void TestFile::fillCheckLines() {    
      
  std::ifstream testFile(testPath);
  if (!testFile.is_open()) {
      std::cerr << "Error opening file: " << testPath << std::endl;
      return;
  }

  std::string line;
  while (std::getline(testFile, line)) {
      size_t findIdx = line.find(check_directive);
      if (findIdx != std::string::npos) {
        std::string checkLine = line.substr(findIdx + strlen(check_directive));
        checkLines.push_back(checkLine);
        hasCheck = true;
      }
  }
  if (!hasCheck) {
    checkLines.push_back("");
  }
  testFile.close();
}
} // namespace tester