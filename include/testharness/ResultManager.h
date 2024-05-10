#ifndef TESTER_RESULT_MANAGER_H
#define TESTER_RESULT_MANAGER_H

#include "tests/TestResult.h"

#include <map>
#include <vector>
#include <string>

namespace tester {

// TODO: make this a set instead of a vector. Theoretically, the uniqueness of the key + filename
// passed in is guaranteed by the caller, but we could enforce it here.
typedef std::vector<TestResult> ResultList;
typedef std::map<std::string, ResultList> ResultSet;
typedef std::map<std::string, ResultSet> ToolChainMap;
typedef std::map<std::string, ToolChainMap> ExecutableMap;

class ResultManager {
public:
  // Add a test result.
  void addResult(const std::string &exe, const std::string &toolchain, const std::string &key,
                 const TestResult &result) {
    results[exe][toolchain][key].push_back(result);
  }

  // Get test results.
  const ResultSet &getResults(const std::string &exe, const std::string &toolchain) const {
    // ToolChainMap tcm = results.at(exe);
    // if (tcm) {
      // tcm.at(toolchain);
    // }
    std::cout << results.size() << std::endl;
    return results.at(exe).at(toolchain);
    
    // return results.at(exe).at(toolchain);
  }

private:
  ExecutableMap results;
};

}

#endif //TESTER_RESULT_MANAGER_H
