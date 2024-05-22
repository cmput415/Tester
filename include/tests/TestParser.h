#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "TestFile.h"
#include "Constants.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace tester {

class TestParser {
public:
  TestParser(TestFile &testfile) 
    : testfile(testfile), foundInput(false), foundInputFile(false), 
      foundCheck(false), insByteCount(0)
  {
#if defined(DEBUG)
  // std::cout << "Constructing Test Parser" << std::endl;
#endif
    parseTest();
  };

  int parseTest();
  void validate();

private:
  // Testfile we parse on
  TestFile &testfile;
    
  // track state of parse
  bool foundInput, foundInputFile, foundCheck;

  // current input stream size
  uint32_t insByteCount;

  // match methods
  ErrorState matchInputDirective(std::string &line);
  ErrorState matchCheckDirective(std::string &line);
  ErrorState matchInputFileDirective(std::string &line);
  ErrorState matchDirectives(std::string &line);
};

}; // namespace tester

#endif // TEST_PARSER_H