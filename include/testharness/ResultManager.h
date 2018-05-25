#ifndef TESTER_RESULT_MANAGER_H
#define TESTER_RESULT_MANAGER_H

#include "testharness/TestResult.h"

#include <map>
#include <vector>
#include <string>
#include <experimental/filesystem>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

// TODO: make this a set instead of a vector. Theoretically, the uniqueness of the key + filename
// passed in is guaranteed by the caller, but we could enforce it here.
typedef std::vector<TestResult> ResultList;
typedef std::map<std::string, ResultList> ResultSet;

class ResultManager {
public:
  // Add a test result.
  void addResult(std::string key, const TestResult &result) { results[key].push_back(result); }

private:
  ResultSet results;
};

}

#endif //TESTER_RESULT_MANAGER_H
