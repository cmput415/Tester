#include "tests/TestParser.h"

namespace tester {

/**
 * @param str the line being operated on
 * @param substr the line to determine if exists in str fully
 * @returns true if substr is fully within str 
*/
bool fullyContains(const std::string &str, const std::string &substr) {
    size_t pos = str.find(substr);
    if (pos == std::string::npos)
        return false;
    return str.substr(pos, substr.length()) == substr; 
}

/**
 * @param line a single line from the test file to parse
 * @param directive which directive we attempt to match
 * @returns a std::variant of either a path or an error type 
*/
PathOrError TestParser::parsePathFromLine(
    const std::string &line,
    const std::string &directive
) {
    size_t findIdx = line.find(directive);
    if (findIdx == std::string::npos) {
        return ErrorState::FileError;
    }

    std::string parsedFilePath = line.substr(findIdx + directive.length());
    fs::path relPath = testfile.getTestPath().parent_path() / fs::path(parsedFilePath);
    fs::path absPath(parsedFilePath);

    if (fs::exists(absPath))
        return absPath;
    else if (fs::exists(relPath)) {
        return relPath;
    } else {
        return ErrorState::FileError;
    }  
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists 
*/
ErrorState TestParser::matchInputDirective(std::string &line) {
    
    if (!fullyContains(line, Directive::INPUT))
        return ErrorState::NoError;    
    if (foundInputFile)
        return ErrorState::DirectiveConflict; // already found an INPUT_FILE
     
    std::ofstream ins(testfile.getInsPath(), std::ios::app);  
    if (!ins.is_open()) 
        return ErrorState::FileError;
    
    size_t findIdx = line.find(Directive::INPUT);
    std::string input =  line.substr(findIdx + Directive::INPUT.length());
    insByteCount += input.length();
    
    if (insByteCount > Directive::MAX_INPUT_BYTES) {
        return ErrorState::MaxInputStreamExceeded;
    }
    
    ins << input << std::endl; // implicit newline 
    foundInput = true; 

    return ErrorState::NoError;
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists 
*/
ErrorState TestParser::matchCheckDirective(std::string &line) {

    if (!fullyContains(line, Directive::CHECK))
        return ErrorState::NoError;
    if (foundCheckFile)
        return ErrorState::DirectiveConflict;
    
    std::ofstream out(testfile.getOutPath(), std::ios::app);  
    if (!out.is_open()) 
        return ErrorState::FileError;

    size_t findIdx = line.find(Directive::CHECK);
    std::string checkLine = line.substr(findIdx + Directive::CHECK.length());

    if (fs::file_size(testfile.getOutPath()) != 0) {
        out << '\n';
    }
    out << checkLine; 
    
    foundCheck = true;
    
    return ErrorState::NoError;
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
*/
ErrorState TestParser::matchInputFileDirective(std::string &line) {

    if (!fullyContains(line, Directive::INPUT_FILE))
        return ErrorState::NoError;
    if (foundInput)
        return ErrorState::DirectiveConflict;

    PathOrError pathOrError = parsePathFromLine(line, Directive::INPUT_FILE);
    if (std::holds_alternative<fs::path>(pathOrError)) {
        testfile.setInsPath(std::get<fs::path>(pathOrError));
        foundInputFile = true;
        return ErrorState::NoError;
    } 
    return std::get<ErrorState>(pathOrError); 
}

/**
 * @param line the line from testfile being parsed
 * @returns An error state describing the error, if one exists
*/
ErrorState TestParser::matchCheckFileDirective(std::string &line) {

    if (!fullyContains(line, Directive::CHECK_FILE))
        return ErrorState::NoError;
    if (foundCheck)
        return ErrorState::DirectiveConflict;

    PathOrError pathOrError = parsePathFromLine(line, Directive::CHECK_FILE);
    if (std::holds_alternative<ErrorState>(pathOrError))
        return std::get<ErrorState>(pathOrError);

    fs::path checkFilePath = std::get<fs::path>(pathOrError);
    std::ifstream checkFileStream(checkFilePath);
    if (!checkFileStream.is_open())
        return ErrorState::FileError;

    std::string checkLine; 
    while (std::getline(checkFileStream, checkLine)) {
        testfile.pushCheckLine(checkLine);
    }
    foundCheckFile = true;
    return ErrorState::NoError;
}

/**
 * @brief for each line in the testfile, attempt to parse and match one of the
 * several directives. Should only be called if the parser knows we are in a comment.
*/
ErrorState TestParser::matchDirectives(std::string &line) {
    ErrorState error;

    // Look for each of 4 directives 
    if ((error = matchInputDirective(line)) != ErrorState::NoError) return error;
    if ((error = matchCheckDirective(line)) != ErrorState::NoError) return error;
    if ((error = matchInputFileDirective(line)) != ErrorState::NoError) return error;
    if ((error = matchCheckFileDirective(line)) != ErrorState::NoError) return error;
    
    return ErrorState::NoError;
}


/**
 * @brief Alter the reference to line to be the substring of itself that is
 * contained with in a comment. Use comment state in class instance to track.   
 */
void TestParser::trackCommentState(std::string &line) {

    std::string result;
    inLineComment = false; // reset line comment

    for (unsigned int i = 0; i < line.length(); i++) {

        if (!inString && !inBlockComment && (i + 1) < line.length()
                      && line[i] == '/' && line[i + 1] == '/') {
            inLineComment = true;
            result += line.substr(i + 2);
            break;
        } 
        else if (!inString && !inLineComment && (i + 1) < line.length() 
                           && line[i] == '/' && line[i + 1] == '*') {
            inBlockComment = true;
            ++i; // skip the * in '/*'
            continue;
        }
        else if (!inString && inBlockComment && (i + 1) < line.length()
                           && line[i] == '*' && line[i + 1] == '/') {
            inBlockComment = false;
            ++i; // skip the / in '*/'
            continue;
        }
        else if (line[i] == '"' && !inBlockComment && (!inString || (i > 0 && line[i - 1] != '\\'))) {
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
 * @brief open up the testfile and begin matching directives in each line, updating
 * the state of the testfile with resource paths and other useful data. 
*/
int TestParser::parseTest() {
    std::ifstream testFileStream(testfile.getTestPath());
    if (!testFileStream.is_open()) {
        std::cerr << "Failed to open the testfile" << std::endl; 
        return -1;
    }

    std::string line;
    while (std::getline(testFileStream, line)) {
        
        trackCommentState(line);
         
        if (!line.empty()) {
            ErrorState error = matchDirectives(line);
            if (error != ErrorState::NoError) {
                std::cout << "Found Error: " << error << std::endl; 
                testfile.setErrorState(error);
                testfile.setErrorMsg("Generic Error"); 
                break;
            }
        } 
    }
    if (!foundCheck && !foundCheckFile) {
        testfile.pushCheckLine(std::move(""));
    }
    if (foundInput || foundInputFile) {
        testfile.usesInputStream = true;
    }
    if (foundInputFile) {
        testfile.usesInputFile = true;
    }

    testFileStream.close();
    return 0; 
}

} // namespace tester