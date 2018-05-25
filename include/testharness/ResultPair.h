#ifndef TESTER_RESULTPAIR_H
#define TESTER_RESULTPAIR_H

#include <experimental/filesystem>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

struct ResultPair {
  // No default constructor.
  ResultPair() = delete;

  // Make the result. Extract the test file name from the path.
  ResultPair(fs::path in, bool pass) : name(in.stem()), pass(pass) { }

  // Info about result.
  const fs::path name;
  const bool pass;
};

} // End namespace tester

#endif //TESTER_RESULTPAIR_H
