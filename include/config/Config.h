#ifndef TESTER_CONFIG_H
#define TESTER_CONFIG_H

#include "toolchain/ToolChain.h"

#include <experimental/filesystem>
#include <map>
#include <string>

// Convenience.
namespace fs = std::experimental::filesystem;

namespace tester {

typedef std::map<std::string, fs::path> Executables;
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
  const fs::path &getInDirPath() const {return inDirPath; }
  const fs::path &getOutDirPath() const {return outDirPath; }

  // Config map getters and convenience indiviual getters.
  const Executables getExecutables() const { return executables; }
  const fs::path &getExecutablePath(const std::string &name) const { return executables.at(name); }
  bool hasExecutable(const std::string &name) const { return executables.count(name) != 0; }

  const ToolChains &getToolChains() const { return toolchains; }
  const ToolChain &getToolChain(const std::string &name) const { return toolchains.at(name); }

  // Config bool getters.
  bool isQuiet() const { return quiet; }

private:
  // Option file paths.
  fs::path gradeFilePath;
  fs::path summaryFilePath;
  fs::path inDirPath;
  fs::path outDirPath;

  // Option file maps.
  Executables executables;
  ToolChains toolchains;

  // Option flags.
  bool quiet;
};

} // End namespace tester

#endif //TESTER_CONFIG_H
