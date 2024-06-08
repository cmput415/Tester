#ifndef TESTER_EXECUTION_STATE_H
#define TESTER_EXECUTION_STATE_H

#include <filesystem>

namespace fs = std::filesystem;

namespace tester {

// A class meant to share intermediate info when starting a new toolchain step.
class ExecutionInput {
public:
  // No default constructor.
  ExecutionInput() = delete;

  // Creates input to a subprocess execution.
  ExecutionInput(fs::path inputPath, fs::path inputStreamPath,
                 fs::path testedExecutable, fs::path testedRuntime)
      : inputPath(std::move(inputPath)),
        inputStreamPath(std::move(inputStreamPath)),
        testedExecutable(std::move(testedExecutable)),
        testedRuntime(std::move(testedRuntime)) {}

  // Gets input file.
  const fs::path& getInputFile() const { return inputPath; }

  // Gets the input stream file.
  const fs::path& getInputStreamFile() const { return inputStreamPath; }

  // Gets tested executable.
  const fs::path& getTestedExecutable() const { return testedExecutable; }

  // Gets tested runtime.
  const fs::path& getTestedRuntime() const { return testedRuntime; }

private:
  fs::path inputPath;
  fs::path inputStreamPath;
  fs::path testedExecutable;
  fs::path testedRuntime;
};

// A class meant to share intermediate info when a toolchain step ends.
class ExecutionOutput {
public:
  // No default constructor.
  ExecutionOutput() = delete;

  // Creates output to a subprocess execution.
  explicit ExecutionOutput(fs::path outputPath, fs::path errorPath = "")
      : outputPath(std::move(outputPath)), errorPath(std::move(errorPath)),
        rv(0) {}

  // Gets output file.
  fs::path getOutputFile() const { return outputPath; }
  fs::path getErrorFile() const { return errorPath; }

  int getReturnValue() const { return rv; }
  void setReturnValue(int returnValue) { rv = returnValue; }

  double elapsedTime; // time executable took to run (seconds)

private:
  fs::path outputPath;
  fs::path errorPath;
  int rv;
};

} // End namespace tester

#endif // TESTER_EXECUTION_STATE_H
