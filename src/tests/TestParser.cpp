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

ParseError copyFile(const fs::path& from, const fs::path& to) {

  // Open the files to operate upon 
  std::ifstream sourceFile(from, std::ios::binary); 
  std::ofstream destFile(to, std::ios::binary);

  // Check for errors opening 
  if (!destFile || !sourceFile) {
    return ParseError::FileError;
  }

  // Write the contents and check for errors
  destFile << sourceFile.rdbuf();
  if (sourceFile.fail() || destFile.fail()) {
    return ParseError::FileError;
  }

  return ParseError::NoError;
} 

void TestParser::insLineToFile(fs::path filePath, std::string line, bool firstInsert) {

  // Set the mode to open the file determined by first insert
  std::ios_base::openmode mode;
  if (firstInsert) {
    mode = std::ios::out | std::ios::trunc;
  } else {
    mode = std::ios::app;
  }

  // Open the file, write the contents.
  std::ofstream out(filePath, mode);
  out << line;
  if (!firstInsert) {
      out << "\n";
  }
  out.close();
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
  
  insLineToFile(testfile->getInsPath(), inputLine, !foundInput);
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
  
  insLineToFile(testfile->getOutPath(), checkLine, !foundCheck);
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

  PathOrError path = parsePathFromLine(line, Directive::INPUT_FILE);
  if (std::holds_alternative<fs::path>(path)) {
    // Copy the input file referenced into the testfiles ephemeral ins file 
    auto inputPath = std::get<fs::path>(path);
    copyFile(inputPath, testfile->getInsPath());
    foundInputFile = true;
  }

  return std::get<ParseError>(path);
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

  PathOrError path = parsePathFromLine(line, Directive::CHECK_FILE);
  if (std::holds_alternative<fs::path>(path)) {
    // Copy the input file referenced into the testfiles ephemeral ins file 
    auto outputPath = std::get<fs::path>(path);
    copyFile(outputPath, testfile->getOutPath());
    foundCheckFile= true;
    return ParseError::NoError;
  }

  return std::get<ParseError>(path);
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
        testfile->setParseError(error);
        testfile->setParseErrorMsg("Generic Error");
        break;
      }
    }
  }

  testFileStream.close();
}

} // namespace tester