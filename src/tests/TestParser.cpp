#include "tests/TestParser.h"

namespace tester {

// returns true iff substring is contained fully within str
bool fullyContains(const std::string &str, const std::string &substr) {
    size_t pos = str.find(substr);
    if (pos == std::string::npos)
        return false;
    return str.substr(pos, substr.length()) == substr; 
}

ErrorState TestParser::matchInputDirective(std::string &line) {

    std::ofstream ins(testfile.getInsPath(), std::ios::app);  
    if (!ins.is_open()) { 
        return ErrorState::FileError;    
    }
    else if (foundInputFile) {
        return ErrorState::DirectiveConflict; // already found an INPUT_FILE
    }
    else if (!fullyContains(line, Constants::INPUT_DIRECTIVE)) {
        return ErrorState::NoError; // directive not found
    } 

    size_t findIdx = line.find(Constants::INPUT_DIRECTIVE);
    std::string input =  line.substr(findIdx + Constants::INPUT_DIRECTIVE.length());
    insByteCount += input.length();

    if (insByteCount > Constants::MAX_INPUT_BYTES) {
        return ErrorState::MaxInputStreamExceeded;
    }
    
    ins << input << std::endl; // implicit newline 
    foundInput = true; 

    return ErrorState::NoError;
}

ErrorState TestParser::matchCheckDirective(std::string &line) {

    if (!fullyContains(line, Constants::CHECK_DIRECTIVE)) {
        return ErrorState::NoError;
    }

    size_t findIdx = line.find(Constants::CHECK_DIRECTIVE);
    std::string checkLine = line.substr(findIdx + Constants::CHECK_DIRECTIVE.length());
    testfile.pushCheckLine(std::move(checkLine));
    foundCheck = true;
    
    return ErrorState::NoError;
}

ErrorState TestParser::matchInputFileDirective(std::string &line) {
    if (foundInput) {
        return ErrorState::DirectiveConflict;
    }

    size_t findIdx = line.find(Constants::INPUT_DIRECTIVE);
    if (findIdx != std::string::npos) {
        foundInputFile = true; 
        std::string parsedFileStr = 
            line.substr(findIdx + Constants::INPUT_FILE_DIRECTIVE.length());
        
        fs::path relPath = testfile.getTestPath().parent_path() / fs::path(parsedFileStr);
        fs::path absPath(parsedFileStr);

        if (fs::exists(absPath)) {
            testfile.setInsPath(absPath);
        } else if (fs::exists(relPath)) {
            testfile.setInsPath(relPath);
        } else {
            return ErrorState::FileError;
        }
    }
    return ErrorState::NoError;
}

ErrorState TestParser::matchDirectives(std::string &line) {
    ErrorState error;
    if ((error = matchInputDirective(line)) != ErrorState::NoError) return error;
    if ((error = matchCheckDirective(line)) != ErrorState::NoError) return error;
    if ((error = matchInputFileDirective(line)) != ErrorState::NoError) return error;
    return ErrorState::NoError;
}

int TestParser::parseTest() {
    std::ifstream testFileStream(testfile.getTestPath());
    if (!testFileStream.is_open()) {
        std::cout << "Failed to open the testfile" << std::endl; 
        return -1;
    }

    std::string line;
    while (std::getline(testFileStream, line)) {
        // updateCommentStack(line);
        // if (inComment(line)) {
        if (1) {
            ErrorState error = matchDirectives(line);
            if (error != ErrorState::NoError) {
                testfile.setErrorState(error);
                testfile.setErrorMsg("Generic Error"); 
                break;
            }
        } 
    }
    if (!foundCheck) {
        testfile.pushCheckLine(std::move(""));
    }
    if (foundInput || foundInputFile) {
        testfile.usesInputStream = true;
    }

    testFileStream.close();
    return 0; 
}

} // namespace tester