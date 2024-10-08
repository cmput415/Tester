#include "config/Config.h"

#include "util.h"

#include "CLI11.hpp"

#include "json.hpp"

#include <iostream>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

Config::Config(int argc, char** argv) : timeout(2l) {

  CLI::App app{"CMPUT 415 testing utility"};

  std::string configFilePath;
  app.add_option("configFile", configFilePath, "Path to the tester JSON configuration file.")
      ->required()
      ->check(CLI::ExistingFile);

  CLI::Option* gradeOpt =
      app.add_option("--grade", gradeFilePath, "Perform grading analysis and output to this file");
  
  CLI::Option* solutionFailureLogOpt =
      app.add_option("--log-failures", failureLogPath, "Log the testcases the solution compiler fails.");

  app.add_option("--timeout", timeout, "Specify timeout length for EACH command in a toolchain.");
  app.add_option("--debug-package", debugPackage, "Provide a sub-path to run the tester on.");
  app.add_flag("-t,--time", time, "Include the timings (seconds) of each test in the output.");
  app.add_flag_function("-v", [&](size_t count) { verbosity = static_cast<int>(count); },
                        "Increase verbosity level");
  
  // Enforce that if a grade path is supplied, then a log file should be as well and vice versa
  gradeOpt->needs(solutionFailureLogOpt);
  solutionFailureLogOpt->needs(gradeOpt);

  // Parse our command line options. This has the potential to throw
  // CLI::ParseError, but we want it to continue up the tree.
  try {
    app.parse(argc, argv);
    initialised = true;
    errorCode = 0;
  } catch (const CLI::Error& e) {
    initialised = false;
    errorCode = app.exit(e);
    return;
  }

  // Get our json file.
  std::ifstream jsonFile(configFilePath);
  JSON json;
  jsonFile >> json;

  // Make sure an in and out dir were provided.
  ensureContains(json, "testDir");
  std::string testDirStr = json["testDir"];
  testDirPath = testDirStr;
  // Ensure the paths exist.
  if (!fs::exists(testDirPath) || !fs::is_directory(testDirPath))
    throw std::runtime_error("Output file directory did not exist: " + testDirStr);

  // Make sure we have an executable to test then set it. Need to explicitly
  // tell json what type we're pulling out here because it doesn't like
  // loading into an fs::path.
  ensureContains(json, "testedExecutablePaths");
  const JSON& tepJson = json["testedExecutablePaths"];
  if (!tepJson.is_object())
    throw std::runtime_error("Tested executable paths was not an object.");

  for (auto it = tepJson.begin(); it != tepJson.end(); ++it) {
    std::string path = it.value();
    executables.emplace(std::make_pair(it.key(), path));
  }

  // Parse out "solutionExecutable": "<name of solution exe>"
  if (doesContain(json, "solutionExecutable")) {
    solutionExecutable = json["solutionExecutable"];
  } else {
    if (gradeOpt->count() == 1) {
      throw std::runtime_error("Must provide a solutionExecutable value when running in grade mode.");
    }
  }
  
  // Add runtimes to the config.
  if (doesContain(json, "runtimes")) {
    const JSON& runtimesJson = json["runtimes"];
    if (!runtimesJson.is_object())
      throw std::runtime_error("Runtime paths was not an object.");

    for (auto it = runtimesJson.begin(); it != runtimesJson.end(); ++it) {
      std::string path = it.value();
      runtimes.emplace(std::make_pair(it.key(), path));
    }
  }

  // Make sure toolchains are provided then build the set of toolchains.
  ensureContains(json, "toolchains");
  const JSON& tcJson = json["toolchains"];
  if (!tcJson.is_object())
    throw std::runtime_error("Toolchains is not an object.");

  for (auto it = tcJson.begin(); it != tcJson.end(); ++it) {
    toolchains.emplace(std::make_pair(it.key(), ToolChain(it.value(), timeout)));
  }
}

} // namespace tester