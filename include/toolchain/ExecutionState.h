#ifndef TESTER_EXECUTION_STATE_H
#define TESTER_EXECUTION_STATE_H

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace tester {

// A class meant to share intermediate info when starting a new toolchain step.
class ExecutionInput {
public:
  // No default constructor.
  ExecutionInput() = delete;

  // Creates input to a subprocess execution.
  ExecutionInput(fs::path inputPath, fs::path inputStreamPath, fs::path testedExecutable,
                 fs::path testedRuntime)
    : inputPath(std::move(inputPath)), inputStreamPath(std::move(inputStreamPath)),
      testedExecutable(std::move(testedExecutable)), testedRuntime(std::move(testedRuntime)) { }

  // Gets input file.
  const fs::path &getInputFile() const { return inputPath; }

  // Gets the input stream file.
  const fs::path &getInputStreamFile() const { return inputStreamPath; }

  // Gets tested executable.
  const fs::path &getTestedExecutable() const { return testedExecutable; }

  // Gets tested runtime.
  const fs::path &getTestedRuntime() const { return testedRuntime; }

private:
  fs::path inputPath;
  fs::path inputStreamPath;
  fs::path testedExecutable;
  fs::path testedRuntime;
};

// A class meant to share intermediate info when end a toolchain step.
class ExecutionOutput {
public:
  // No default constructor.
  ExecutionOutput() = delete;

  // Creates output to a subprocess execution.
  explicit ExecutionOutput(fs::path outputPath) : outputPath(std::move(outputPath)) { }

  // Gets output file.
  fs::path getOutputFile() const { return outputPath; }

private:
  fs::path outputPath;
};

} // End namespace tester

#endif // TESTER_EXECUTION_STATE_H
