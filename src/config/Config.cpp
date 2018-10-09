#include "config/Config.h"

#include "util.h"

#include "CLI11.hpp"

#include "json.hpp"

//DEBUG
#include <iostream>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

Config::Config(int argc, char **argv) {
  CLI::App app{"CMPUT 415 testing utility"};

  // Add the internal config file path option.
  std::string configFilePath;
  app.add_option("configFile", configFilePath, "Path to the tester JSON configuration file.")
      ->required()->check(CLI::ExistingFile);

  // Add quiet flag to not print out diffs.
  CLI::Option *quietFlag = app.add_flag("-q,--quiet", quiet, "Quiet mode, don't print fail diffs");

  // Set file to dump summary to instead of stdout.
  CLI::Option *summaryOpt = app.add_option("--summary", summaryFilePath,
      "Write the test summary to this file instead of stdout");

  // Set file to put grading output into.
  CLI::Option *gradeOpt = app.add_option("--grade", gradeFilePath,
      "Perform grading analysis and output to this file");

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
    executables.emplace(it.key(), path);
  }

  // Make sure toolchains are provided then build the set of toolchains.
  ensureContains(json, "toolchains");
  const JSON &tcJson = json["toolchains"];
  if (!tcJson.is_object())
    throw std::runtime_error("Toolchains is not an object.");

  for (auto it = tcJson.begin(); it != tcJson.end(); ++it) {
    toolchains.emplace(it.key(), it.value());
  }

  // Make sure an in and out dir were provided.
  ensureContains(json, "inDir");
  ensureContains(json, "outDir");

  // Get the in and out paths.
  std::string inDirStr = json["inDir"];
  std::string outDirStr = json["outDir"];
  inDirPath = inDirStr;
  outDirPath = outDirStr;

  // Ensure the paths exist.
  if (!fs::exists(inDirPath) || !fs::is_directory(inDirPath))
    throw std::runtime_error("Input file directory did not exist: " + inDirStr);
  if (!fs::exists(outDirPath) || !fs::is_directory(outDirPath))
    throw std::runtime_error("Output file directory did not exist: " + outDirStr);

  std::cout << "PATHS: " << inDirPath << " " << outDirPath << '\n';
//  std::cout << "TESTE:"
}

}
