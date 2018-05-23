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
  explicit ExecutionInput(const fs::path inputPath) : inputPath(inputPath) { }

  // Gets input file.
  fs::path getInputFile() const { return inputPath; }

private:
  fs::path inputPath;
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
