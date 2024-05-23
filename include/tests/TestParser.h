#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "TestFile.h"
#include "Constants.h"
#include <filesystem>
#include <variant>

namespace fs = std::filesystem;

namespace tester {

using PathOrError = std::variant<fs::path, ErrorState>;

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
  bool foundInput, foundInputFile, foundCheck, foundCheckFile;

  // current input stream size
  uint32_t insByteCount;

  PathOrError parsePathFromLine(const std::string &line, const std::string &directive);

  // match methods
  ErrorState matchInputDirective(std::string &line);
  ErrorState matchCheckDirective(std::string &line);
  ErrorState matchInputFileDirective(std::string &line);
  ErrorState matchCheckFileDirective(std::string &line);
  ErrorState matchDirectives(std::string &line);
};

}; // namespace tester

#endif // TEST_PARSER_H