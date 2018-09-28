#ifndef TESTER_TEST_PAIR_H
#define TESTER_TEST_PAIR_H

#include <experimental/filesystem>

// Convenience
namespace fs = std::experimental::filesystem;

namespace tester{

// Struct that functions as a better named pair for test files matches.
struct PathPair {
  PathPair() = delete;

  PathPair(fs::path in, fs::path out) : in(in), out(out) { }

  const fs::path in, out;
};

} // End namespace tester
#endif //TESTER_TESTPAIR_H
