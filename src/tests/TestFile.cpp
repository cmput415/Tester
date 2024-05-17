#include "tests/TestFile.h"
#include "tests/TestParser.h" 

namespace tester {

TestFile::TestFile(const fs::path &path) : testPath(path) {

  // set input stream path to tmp
  inputStrPath = fs::temp_directory_path() / testPath.filename().replace_extension(".ins");

  // pass a mutable reference to the Parser
  TestParser parser(*this);

  std::cout << "FILE SIZE: " << fs::file_size(inputStrPath) 
    << std::endl
    << "FILE EXISTS: " << fs::exists(inputStrPath)
    << std::endl; 
}

TestFile::~TestFile() {

  std::error_code ec;
  fs::remove(inputStrPath, ec);
  if (ec) {
    // TODO: whether or not this is right way to handle errors.
    std::cerr << "ERROR: Unable to delete temporary input stream file." << std::endl;
  } else {
    std::cout << "removed file: " << std::endl;  
  }
}

} // namespace tester





// void TestFile::fillInputStreamFile() {
 
//   insPath = fs::temp_directory_path() / testPath.filename().replace_extension(".ins");
//   std::ifstream testFile(testPath);
//   std::ofstream tmpInsFile(insPath);
  
//   // TODO: how to handle errors properly 
//   if (!testFile.is_open()) {
//     std::cerr << "ERROR: failed to open test file: " << testPath << std::endl; 
//     return;
//   } else if (!tmpInsFile.is_open()) {
//     std::cerr << "ERROR: failed to open inStream file: " << insPath << std::endl; 
//     return; 
//   }

//   // TODO: ensure that NEWLINES are not ignored. Should an empty INPUT: generate a string with only a newline character?
//   std::string line;
//   while (std::getline(testFile, line)) {
//     size_t findIdx = line.find(input_directive);
//     if (findIdx != std::string::npos) {
//       // write the conetnts following the INPUT directive into the input stream file.
//       tmpInsFile << line.substr(findIdx + strlen(input_directive)) << std::endl;
//       hasInput = true; 
//       // can not combine INPUT and INPUT_FILE directive in one test
//       continue; 
//     }

//     size_t inputFileIdx = line.find(input_file_directive);
//     if (inputFileIdx != std::string::npos) {
//       std::string insFilePath = line.substr(inputFileIdx + strlen(input_file_directive));
//       std::ifstream insFile(insFilePath);
//       if (!insFile.is_open()) {
//         testErrorMessage = "Failed to open file:"; 
//         badTest = true;
//       }
//       // validate input
      
//       // if the input is valid and the path exists, assign to the insPath member variable
//       insPath = insFilePath;
//     }
//   }
 
//   //DEBUG: print the contents of insFile
//   // std::cout << "DEBUG" << std::endl;
//   // std::ifstream dumpInsFile(insPath);
//   // std::string dumpLine;
//   // while (std::getline(dumpInsFile, dumpLine)) {
//   //   std::cout << dumpLine << std::endl;
//   // } 
//   // std::cout << "END_DEBUG" << std::endl;

//   // Close files
//   tmpInsFile.close();
//   testFile.close();  
// }

// void TestFile::fillCheckLines() {    
      
//   std::ifstream testFile(testPath);
//   if (!testFile.is_open()) {
//       std::cerr << "Error opening file: " << testPath << std::endl;
//       return;
//   }

//   std::string line;
//   while (std::getline(testFile, line)) {
//       size_t findIdx = line.find(check_directive);
//       if (findIdx != std::string::npos) {
//         std::string checkLine = line.substr(findIdx + strlen(check_directive));
//         checkLines.push_back(checkLine);
//         hasCheck = true;
//       }
//   }
//   if (!hasCheck) {
//     checkLines.push_back(""); // Make the tester validate against an empty output
//   }
//   testFile.close();
// }
// } // namespace tester