#ifndef TESTER_CONFIG_H
#define TESTER_CONFIG_H

#include "toolchain/ToolChain.h"

#include <filesystem>
#include <map>
#include <string>

// Convenience.
namespace fs = std::filesystem;

namespace tester {

typedef std::map<std::string, fs::path> PathMap;
typedef std::map<std::string, ToolChain> ToolChains;

class Config {
public:
  // No default constructor.
  Config() = default;

  // Construct with argc/argv.
  Config(int argc, char ** argv);

  // Config path getters and checkers.
  const fs::path &getGradePath() const { return gradeFilePath; }
  bool hasGradePath() const { return !gradeFilePath.empty(); }
  const fs::path &getSummaryPath() const { return summaryFilePath; }
  bool hasSummaryPath() const { return !summaryFilePath.empty(); }
  const fs::path &getTestDirPath() const { return testDirPath; }

  // Config map getters and convenience individual getters.
  const PathMap getExecutables() const { return executables; }
  const fs::path &getExecutablePath(const std::string &name) const { return executables.at(name); }
  bool hasExecutable(const std::string &name) const { return executables.count(name) != 0; }

  const PathMap getRuntimes() const { return runtimes; }
  const fs::path &getRuntimePath(const std::string &name) const { return runtimes.at(name); }
  bool hasRuntime(const std::string &name) const { return runtimes.count(name) != 0; }

  const ToolChains &getToolChains() const { return toolchains; }
  const ToolChain &getToolChain(const std::string &name) const { return toolchains.at(name); }

  // Config bool getters.
  bool isQuiet() const { return quiet; }

  // Config int getters.
  int64_t getTimeout() const { return timeout; }

  // Initialisation verification.
  bool isInitialised() const { return initialised; }
  int getErrorCode() const { return errorCode; }

private:
  // Option file paths.
  fs::path gradeFilePath;
  fs::path summaryFilePath;
  fs::path testDirPath;

  // Option file maps.
  PathMap executables;
  PathMap runtimes;
  ToolChains toolchains;

  // Option flags.
  bool quiet;

  // The command timeout.
  int64_t timeout;

  // Is the config initialised or not and an appropriate error code. This could be due to asking for
  // help or a missing config file.
  bool initialised;
  int errorCode;
};

} // End namespace tester

#endif //TESTER_CONFIG_H
