#ifndef TESTER_EXECUTION_STATE_H
#define TESTER_EXECUTION_STATE_H

#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

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
        testedExecutable(std::move(testedExecutable)), testedRuntime(std::move(testedRuntime)) {}

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
 
  ExecutionOutput() : rv(0), elapsedTime(0), hasElapsed(0), isErrorTest(false) {};

  // Creates output to a subprocess execution.
  explicit ExecutionOutput(fs::path outPath, fs::path errPath)
      : outPath(std::move(outPath)),
        errPath(std::move(errPath)), 
        rv(0), elapsedTime(0), hasElapsed(false), isErrorTest(false) {}

  // Gets output file.
  fs::path getOutputFile() const { return outPath; }
  fs::path getErrorFile() const { return errPath; }

  // Get CPU time
  std::optional<double> getElapsedTime() const {
    return hasElapsed ? std::optional<double>(elapsedTime) : std::nullopt;
  }

  int getReturnValue() const { return rv; }
  
  void setReturnValue(int returnValue) { rv = returnValue; }
  
  void setElapsedTime(double time) {
    elapsedTime = time;
    hasElapsed = true;
  }
 
  void setIsErrorTest(bool errorTest) { isErrorTest = errorTest; }
  bool IsErrorTest() const { return isErrorTest; }

private:
  fs::path outPath;
  fs::path errPath;
  
  int rv;
  
  // time executable took to run (seconds)
  double elapsedTime;
  bool hasElapsed;

  // Flag to indicate if the generated output should be read from stdout or stderr.
  // For error tests we grab from stderr.
  bool isErrorTest;
};

} // End namespace tester

#endif // TESTER_EXECUTION_STATE_H
