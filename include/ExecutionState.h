#ifndef TESTER_EXECUTION_STATE_H
#define TESTER_EXECUTION_STATE_H

#include <string>

namespace tester {

// A class meant to share intermediate info when starting a new toolchain step.
class ExecutionInput {
public:
  // No default constructor.
  ExecutionInput() = delete;

  // Creates input to a subprocess execution.
  ExecutionInput(const std::string inputFile) : inputFile(inputFile) { }

  // Gets input file.
  std::string getInputFile() const { return inputFile; }

private:
  std::string inputFile;
};

// A class meant to share intermediate info when end a toolchain step.
class ExecutionOutput {
public:
  // No default constructor.
  ExecutionOutput()  = delete;

  // Creates output to a subprocess execution.
  ExecutionOutput(std::string outputFile) : outputFile(outputFile) { }

  // Gets output file.
  std::string getOutputFile() const { return outputFile; }


private:
  std::string outputFile;
};

} // End namespace tester

#endif // TESTER_EXECUTION_STATE_H
