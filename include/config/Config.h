#ifndef TESTER_CONFIG_H
#define TESTER_CONFIG_H

#include "toolchain/ToolChain.h"

#include <cstdint>
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
  Config(int argc, char** argv);

  // Config path getters and checkers.
  const std::optional<fs::path>& getGradePath() const { return gradeFilePath; }
  const std::optional<fs::path>& getDebugPath() const { return debugPackage; }
  const std::optional<fs::path>& getFailureLogPath() const { return failureLogPath; };

  // Non optional config variables 
  const fs::path&getTestDirPath() const { return testDirPath; }

  // Config map getters and convenience individual getters.
  const PathMap getExecutables() const { return executables; }
  const fs::path& getExecutablePath(const std::string& name) const { return executables.at(name); }
  bool hasExecutable(const std::string& name) const { return executables.count(name) != 0; }

  const PathMap getRuntimes() const { return runtimes; }
  const fs::path& getRuntimePath(const std::string& name) const { return runtimes.at(name); }
  bool hasRuntime(const std::string& name) const { return runtimes.count(name) != 0; }

  const ToolChains& getToolChains() const { return toolchains; }
  const ToolChain& getToolChain(const std::string& name) const { return toolchains.at(name); }

  // Get solution executable (Shoul only exist in grade mode.)
  std::optional<std::string> getSolutionExecutable() const { return solutionExecutable; }
  
  // Config bool getters.
  bool isTimed() const { return time; }
  bool isMemoryChecked() const { return memory; }
  int getVerbosity() const { return verbosity; }

  // Config int getters.
  int64_t getTimeout() const { return timeout; }
  int64_t getNumThreads() const { return numThreads; }

  // Initialisation verification.
  bool isInitialised() const { return initialised; }
  int getErrorCode() const { return errorCode; }

  
private:
  // Option file paths.
  std::optional<fs::path> gradeFilePath;
  std::optional<fs::path> failureLogPath;
  std::optional<fs::path> debugPackage;

  fs::path testDirPath;

  // Option file maps.
  PathMap executables;
  PathMap runtimes;
  ToolChains toolchains;
  
  // In grader mode, we identify which executable is the solution so we can
  // mark potentially invalid testcases.
  std::optional<std::string> solutionExecutable;
  
  // Option flags.
  bool debug, time, memory;
  int verbosity{0};

  // The command timeout.
  int64_t timeout;

  // Number of threads on which to run tests
  int64_t numThreads;
  // Number of tests for each thread to grab on each run

  // Is the config initialised or not and an appropriate error code. This
  // could be due to asking for help or a missing config file.
  bool initialised;
  int errorCode;
};

} // End namespace tester

#endif // TESTER_CONFIG_H
