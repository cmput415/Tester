#include "tests/TestParser.h"

namespace tester {

/**
 * @param str the line being operated on
 * @param substr the line to determine if exists in str fully
 * @returns true if substr is fully within str
 */
bool fullyContains(const std::string& str, const std::string& substr) {
  size_t pos = str.find(substr);
  if (pos == std::string::npos)
    return false;
  return str.substr(pos, substr.length()) == substr;
}

void TestParser::insLineToFile(fs::path filePath, std::string line, bool firstInsert) {
  // open in append mode since otherwise multi-line checks and inputs would
  // over-write themselves.
  std::ofstream out(filePath, std::ios::app);
  if (!firstInsert) {
    out << "\n";
  }
  out << line;
}

/**
 * @param line a single line from the test file to parse
 * @param directive which directive we attempt to match
 * @returns a std::variant of either a path or an error type
 */
PathOrError TestParser::parsePathFromLine(const std::string& line, const std::string& directive) {
  size_t findIdx = line.find(directive);
  if (findIdx == std::string::npos) {
    return ParseError::FileError;
  }

  std::string parsedFilePath = line.substr(findIdx + directive.length());
  fs::path relPath = testfile->getTestPath().parent_path() / fs::path(parsedFilePath);
  fs::path absPath(parsedFilePath);

  if (fs::exists(absPath))
    return absPath;
  else if (fs::exists(relPath)) {
    return relPath;
  } else {
    return ParseError::FileError;
  }
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
 */
ParseError TestParser::matchInputDirective(std::string& line) {

  if (!fullyContains(line, Directive::INPUT))
    return ParseError::NoError;
  if (foundInputFile)
    return ParseError::DirectiveConflict; // already found an INPUT_FILE

  size_t findIdx = line.find(Directive::INPUT);
  std::string inputLine = line.substr(findIdx + Directive::INPUT.length());

  try {
    insLineToFile(testfile->getInsPath(), inputLine, !foundInput);
  } catch (...) {
    return ParseError::FileError;
  }

  foundInput = true;
  return ParseError::NoError;
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
 */
ParseError TestParser::matchCheckDirective(std::string& line) {

  if (!fullyContains(line, Directive::CHECK))
    return ParseError::NoError;
  if (foundCheckFile)
    return ParseError::DirectiveConflict;

  size_t findIdx = line.find(Directive::CHECK);
  std::string checkLine = line.substr(findIdx + Directive::CHECK.length());

  try {
    insLineToFile(testfile->getOutPath(), checkLine, !foundCheck);
  } catch (...) {
    return ParseError::FileError;
  }

  foundCheck = true;
  return ParseError::NoError;
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
 */
ParseError TestParser::matchInputFileDirective(std::string& line) {

  if (!fullyContains(line, Directive::INPUT_FILE))
    return ParseError::NoError;
  if (foundInput)
    return ParseError::DirectiveConflict;

  PathOrError pathOrError = parsePathFromLine(line, Directive::INPUT_FILE);
  if (std::holds_alternative<fs::path>(pathOrError)) {
    testfile->setInsPath(std::get<fs::path>(pathOrError));
    foundInputFile = true;
    return ParseError::NoError;
  }
  return std::get<ParseError>(pathOrError);
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
 */
ParseError TestParser::matchCheckFileDirective(std::string& line) {

  if (!fullyContains(line, Directive::CHECK_FILE))
    return ParseError::NoError;
  if (foundCheck)
    return ParseError::DirectiveConflict;

  PathOrError pathOrError = parsePathFromLine(line, Directive::CHECK_FILE);
  if (std::holds_alternative<fs::path>(pathOrError)) {
    testfile->setOutPath(std::get<fs::path>(pathOrError));
    foundCheckFile = true;
    return ParseError::NoError;
  }

  return std::get<ParseError>(pathOrError);
}

/**
 * @brief for each line in the testfile, attempt to parse and match one of the
 * several directives. Should only be called if the parser knows we are in a
 * comment.
 */
ParseError TestParser::matchDirectives(std::string& line) {
  ParseError error;

  // Look for each of 4 directives
  if ((error = matchInputDirective(line)) != ParseError::NoError)
    return error;
  if ((error = matchCheckDirective(line)) != ParseError::NoError)
    return error;
  if ((error = matchInputFileDirective(line)) != ParseError::NoError)
    return error;
  if ((error = matchCheckFileDirective(line)) != ParseError::NoError)
    return error;

  return ParseError::NoError;
}

/**
 * @brief Alter the reference to line to be the substring of itself that is
 * contained with in a comment. Use comment state in class instance to track.
 */
void TestParser::trackCommentState(std::string& line) {

  std::string result;
  inLineComment = false; // reset line comment

  for (unsigned int i = 0; i < line.length(); i++) {
    if (!inString && !inBlockComment && (i + 1) < line.length() && line[i] == '/' &&
        line[i + 1] == '/') {
      inLineComment = true;
      result += line.substr(i + 2); // save whatever comes after the line comment
      break;
    } else if (!inString && !inLineComment && (i + 1) < line.length() && line[i] == '/' &&
               line[i + 1] == '*') {
      inBlockComment = true;
      ++i; // skip the * in '/*'
      continue;
    } else if (!inString && inBlockComment && (i + 1) < line.length() && line[i] == '*' &&
               line[i + 1] == '/') {
      inBlockComment = false;
      ++i; // skip the / in '*/'
      continue;
    } else if (line[i] == '"' && !inBlockComment) {
      // check if it was an escaped double qoute
      if ((i > 0 && line[i - 1] == '\\')) {
        continue;
      }
      inString = !inString;
    }
    // while we are in a block comment store characters in result
    if (inBlockComment) {
      result += line[i];
    }
  }
  line = result;
}

/**
 * @brief open up the testfile and begin matching directives in each line,
 * updating the state of the testfile with resource paths and other useful data.
 */
void TestParser::parse() {

  std::ifstream testFileStream(testfile->getTestPath());
  if (!testFileStream.is_open()) {
    std::cerr << "Failed to open the testfile" << std::endl;
    return;
  }

  std::string line;
  while (std::getline(testFileStream, line)) {

    // mutate current line to be only the substring contained in a comment.
    trackCommentState(line);
    if (!line.empty()) {
      ParseError error = matchDirectives(line);
      if (error != ParseError::NoError) {
        testfile->getParseError(error);
        testfile->setParseErrorMsg("Generic Error");
        break;
      }
    }
  }

  // Set final flags to update test state
  testfile->usesInputStream = (foundInput || foundInputFile);
  testfile->usesInputFile = (foundInputFile);
  testfile->usesOutStream = (foundCheck || foundCheckFile);
  testfile->usesOutFile = foundCheckFile;

  // ck if input directives have exceeded maximum
  if (fs::file_size(testfile->getInsPath()) > Directive::MAX_INPUT_BYTES) {
    testfile->getParseError(ParseError::MaxInputBytesExceeded);
  } else if (fs::file_size(testfile->getOutPath()) > Directive::MAX_OUTPUT_BYTES) {
    testfile->getParseError(ParseError::MaxOutputBytesExceeded);
  }

  testFileStream.close();
}

} // namespace tester