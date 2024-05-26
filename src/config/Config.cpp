#include "config/Config.h"

#include "util.h"

#include "CLI11.hpp"

#include "json.hpp"

//DEBUG
#include <iostream>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

Config::Config(int argc, char **argv) : timeout(2l) {
  CLI::App app{"CMPUT 415 testing utility"};

  // ---- These flags are necessary always.
  // Add the internal config file path option.
  std::string configFilePath;
  app.add_option("configFile", configFilePath, "Path to the tester JSON configuration file.")
      ->required()->check(CLI::ExistingFile);

  // ---- These flags are for regular testing.
  // Set file to dump summary to instead of stdout.
  CLI::Option *summaryOpt = app.add_option("--summary", summaryFilePath,
      "Write the test summary to this file instead of stdout");

  // Add quiet flag to not print out diffs.
  CLI::Option *quietFlag = app.add_flag("-q,--quiet", quiet, "Quiet mode, don't print fail diffs");
  CLI::Option *verboseFlag = app.add_flag("-v,--verbose", verbose, "Verbose mode, dump files for failed tests");

  // ---- These flags are for grading.
  // Set file to put grading output into.
  CLI::Option *gradeOpt = app.add_option("--grade", gradeFilePath,
      "Perform grading analysis and output to this file");

  // ---- These flags are optional always.
  // Add a timeout flag that defaults to 2 seconds (see initialiser list).
  app.add_option("--timeout", timeout, "Specify timeout length for EACH command in a toolchain.");

  // If we're doing grading then it doesn't make sense to specify quiet or summary.
  gradeOpt->excludes(quietFlag)->excludes(summaryOpt);

  // Parse our command line options. This has the potential to throw CLI::ParseError, but we want it
  // to continue up the tree.
  try {
    app.parse(argc, argv);
    initialised = true;
    errorCode = 0;
  }
  catch(const CLI::Error &e) {
    initialised = false;
    errorCode = app.exit(e);
    return;
  }

  // Get our json file.
  std::ifstream jsonFile(configFilePath);
  JSON json;
  jsonFile >> json;

  // Make sure we have an executable to test then set it. Need to explicitly tell json what type
  // we're pulling out here because it doesn't like loading into an fs::path.
  ensureContains(json, "testedExecutablePaths");
  const JSON &tepJson = json["testedExecutablePaths"];
  if (!tepJson.is_object())
    throw std::runtime_error("Tested executable paths was not an object.");


  for (auto it = tepJson.begin(); it != tepJson.end(); ++it) {
    std::string path = it.value();
    executables.emplace(std::make_pair(it.key(), path));
  }

  // Make sure toolchains are provided then build the set of toolchains.
  ensureContains(json, "toolchains");
  const JSON &tcJson = json["toolchains"];
  if (!tcJson.is_object())
    throw std::runtime_error("Toolchains is not an object.");

  for (auto it = tcJson.begin(); it != tcJson.end(); ++it) {
    toolchains.emplace(std::make_pair(it.key(), ToolChain(it.value(), timeout)));
  }

  // Make sure an in and out dir were provided.
  ensureContains(json, "testDir");
  std::string testDirStr = json["testDir"];
  testDirPath = testDirStr;

  // Ensure the paths exist.
  if (!fs::exists(testDirPath) || !fs::is_directory(testDirPath))
    throw std::runtime_error("Output file directory did not exist: " + testDirStr);
  
  // Add runtimes to the config.
  if (doesContain(json, "runtimes")) {
    const JSON &runtimesJson = json["runtimes"];
    if (!runtimesJson.is_object())
      throw std::runtime_error("Runtime paths was not an object.");

    for (auto it = runtimesJson.begin(); it != runtimesJson.end(); ++it) {
      std::string path = it.value();
      runtimes.emplace(std::make_pair(it.key(), path));
    }

  }
}

}
