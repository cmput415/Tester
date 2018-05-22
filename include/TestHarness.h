#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "toolchain/ToolChain.h"

#include "json.hpp"

#include <string>
#include <vector>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;
namespace fs = std::experimental::filesystem;

namespace tester {

// Test file typedefs.
struct TestPair; // Definition below.
typedef std::vector<TestPair> TestList;
typedef std::map<std::string, TestList> TestSet;

// Class that manages finding tests and running them.
class Tester {
public:
  // No default constructor.
  Tester() = delete;

  // Construct the Tester with a parsed JSON file.
  Tester(const JSON &json);

  // Run the found tests.
  void runTests() { }

private:
  ToolChain toolchain;
  TestSet tests;
};

// Implement TestPair.
struct TestPair {
  TestPair() = delete;

  TestPair(fs::path in, fs::path out) : in(in), out(out) { }

  const fs::path in, out;
};

} // End namespace tester

#endif //TESTER_TESTER_H
