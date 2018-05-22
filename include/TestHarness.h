#ifndef TESTER_TEST_HARNESS_H
#define TESTER_TEST_HARNESS_H

#include "toolchain/TestPair.h"
#include "toolchain/ToolChain.h"

#include "json.hpp"

#include <string>
#include <vector>
#include <experimental/filesystem>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// Test file typedefs.
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

} // End namespace tester

#endif //TESTER_TESTER_H
