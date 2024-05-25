#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "TestFile.h"
#include "Directives.h"
#include <filesystem>
#include <variant>

namespace fs = std::filesystem;

namespace tester {

using PathOrError = std::variant<fs::path, ErrorState>;

class TestParser {
public:
  TestParser(TestFile &testfile) 
    : testfile(testfile), foundInput(false), foundInputFile(false), 
      foundCheck(false), foundCheckFile(false), inLineComment(false), 
      inBlockComment(false), inString(false), insByteCount(0)
  {
    parseTest();
  };

  int parseTest();
  void validate();

private:
  // Testfile we parse on
  TestFile &testfile;
    
  // track state of parse
  bool foundInput, foundInputFile, foundCheck, foundCheckFile;

  // track comment state
  bool inLineComment, inBlockComment, inString; 

  // current input stream size
  uint32_t insByteCount;

  // determine if we are in a comment while parsing
  void trackCommentState(std::string &line); 
  
  // helper method to return the path in a FILE directive if it is good
  PathOrError parsePathFromLine(const std::string &line, const std::string &directive);

  // methods below look for INPUT, CHECK, INPUT_FILE, CHECK_FILE directive in a lines  
  ErrorState matchInputDirective(std::string &line); 
  ErrorState matchCheckDirective(std::string &line);
  ErrorState matchInputFileDirective(std::string &line);
  ErrorState matchCheckFileDirective(std::string &line);
  ErrorState matchDirectives(std::string &line);

};

}; // namespace tester

#endif // TEST_PARSER_H