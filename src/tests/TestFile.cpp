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
 
  fs::path tempDir = fs::temp_directory_path();
  fs::path tempFile = testPath.replace_extension(".ins");  
  insPath = tempDir / tempFile;
  
  // read the .test file to parse directives and write to .ins file the found stdin
  std::ifstream inFile(testPath);
  std::ofstream insFile(insPath);

  if (!inFile.is_open() || !insFile.is_open()) {
    // TODO: Throw proper error here
    std::cerr << "ERROR" << std::endl;
    return;
  } 

  std::string line;
  while (std::getline(inFile, line)) {
    size_t findIdx = line.find(input_directive);
    if (findIdx != std::string::npos) {
      // write the conetnts following the INPUT directive into the input stream file.
      insFile << line.substr(findIdx + strlen(input_directive)) << std::endl;
      hasInputStream = true; 
    }
  }
  // Close files
  insFile.close();
  inFile.close();  
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
      }
  }
  testFile.close();
  }
} // namespace tester