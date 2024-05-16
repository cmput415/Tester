#ifndef TEST_FILE_PARSER_H
#define TEST_FILE_PARSER_H

#include "tests/TestFile.h"
#include <stack>

// Track multiline comments
enum CommentToken {
    OpenComment = 0,
    CloseComment = 1
};

namespace tester {

class TestFileParser {
public:
    TestFileParser(TestFile& testfile);

    // Compute the final state of testFile 
    void parse() {};

private:
    std::stack<CommentToken> commentStack;
    TestFile &testfile;

    // returns true if we are in a comment.
    bool inComment() { return !commentStack.empty(); };
};

} // namespace tester

#endif