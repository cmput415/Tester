#ifndef TESTER_TEST_RESULTS_H
#define TESTER_TEST_RESULTS_H

#include "ResultPair.h"

#include <map>
#include <vector>
#include <string>
#include <experimental/filesystem>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

// TODO: make this a set instead of a vector. Theoretically, the uniqueness of the key + filename
// passed in is guaranteed by the caller, but we could enforce it here.
typedef std::vector<ResultPair> ResultList;
typedef std::map<std::string, ResultList> ResultSet;

class TestResults {

  // Add a test result.
  void addResult(std::string key, fs::path in, bool pass) { results[key].emplace_back(in, pass); }

private:
  ResultSet results;
};

}

#endif //TESTER_TESTRESULTS_H
