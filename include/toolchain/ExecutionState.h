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
  ExecutionInput(const fs::path &inputPath, const fs::path &testedExecutable)
    : inputPath(inputPath), testedExecutable(testedExecutable) { }

  // Gets input file.
  fs::path getInputFile() const { return inputPath; }

  // Gets tested executable.
  fs::path getTestedExecutable() const { return testedExecutable; }

private:
  fs::path inputPath;
  fs::path testedExecutable;
};

// A class meant to share intermediate info when end a toolchain step.
class ExecutionOutput {
public:
  // No default constructor.
  ExecutionOutput()  = delete;

  // Creates output to a subprocess execution.
  ExecutionOutput(fs::path outputPath) : outputPath(outputPath) { }

  // Gets output file.
  fs::path getOutputFile() const { return outputPath; }

private:
  fs::path outputPath;
};

} // End namespace tester

#endif // TESTER_EXECUTION_STATE_H
