#ifndef TESTER_TEST_RESULT_H
#define TESTER_TEST_RESULT_H

#include <experimental/filesystem>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

struct TestResult {
  // No default constructor.
  TestResult() = delete;

  // Make the result. Extract the test file name from the path.
  TestResult(fs::path in, bool pass, bool error, std::string diff)
    : name(in.stem()), pass(pass), error(error), diff(diff) { }

  // Info about result.
  const fs::path name;
  const bool pass;
  const bool error;
  const std::string diff;
};

} // End namespace tester

#endif //TESTER_TEST_RESULT_H
