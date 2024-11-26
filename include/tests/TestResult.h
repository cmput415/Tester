#ifndef TESTER_TEST_RESULT_H
#define TESTER_TEST_RESULT_H

#include <filesystem>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

struct TestResult {
  // No default constructor.
  TestResult() = delete;

  // Make the result. Extract the test file name from the path.
  TestResult(fs::path in, bool pass, bool error, std::string diff)
      : name(in.stem()), pass(pass), error(error), diff(diff) {}

  // Info about result.
  fs::path name;
  bool pass;
  bool error;
  std::string diff;

  TestResult clone() { return TestResult(this->name, this->pass, this->error, this->diff); }

  void swap(TestResult &other) {
    std::swap(name, other.name);
    std::swap(pass, other.pass);
    std::swap(error, other.error);
    std::swap(diff, other.diff);
  }

};
} // End namespace tester

#endif // TESTER_TEST_RESULT_H
