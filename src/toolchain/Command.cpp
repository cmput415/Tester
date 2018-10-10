#include "toolchain/Command.h"

#include "util.h"

#include "toolchain/CommandException.h"

#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#if __linux__
#include <wait.h>
#elif __APPLE__
#include <signal.h>
#include <sys/wait.h>
#endif

namespace {

// This get a bit complicated. We want easy command running which is available in a cross platform
// manner via std::system but there's no way for us to kill a long running subprocess (i.e. there's
// and infinite loop in a test). This means we need to fall back on forking/execing, unfortunately.
void runCommand(std::promise<unsigned int> &promise, std::atomic_bool &killVar,
                const std::string &exe, const std::vector<std::string> &trueArgs,
                const std::string &output) {
  // Build a list of true arguments.
  const char *args[trueArgs.size() + 2];

  // The base arg is the executable.
  args[0] = exe.c_str();

  // Now fill the actual args in.
  for (size_t i = 0; i < trueArgs.size(); ++i)
    args[i + 1] = trueArgs[i].c_str();

  // Null terminate the args array.
  args[trueArgs.size() + 1] = NULL;

  // Do the actual fork.
  pid_t childId = fork();

  // We're the child process, we want to replace our process image with the shell running the
  if (childId == 0) {
    // If we're provided with a redirect file we need to replace STDOUT.
    if (!output.empty()) {
      // Open up the file we'd like to use. Write only, create if it doesn't exist, truncate if it
      // does exist. User can read and write after.
      int fd = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

      // Close the stream behind STDOUT and replace it with the file we opened.
      dup2(fd, STDOUT_FILENO);

      // Close the descriptor to that file.
      close(fd);
    }

    // Replace ourself with the program
    execv(exe.c_str(), const_cast<char * const *>(args));

    // Because execv replaces the current process, we only ever get here if it fails.
    perror("waitpid");
    throw std::runtime_error("Child process didn't start.");
  }

  // We're in the parent process. Set up variables for watching the child process.
  int status;
  pid_t closing;

  // Initial attempt to wait.
  closing = waitpid(childId, &status, WNOHANG);

  // Our busy loop, continually asking about the status of the child.
  while (closing == 0 && !killVar.load()) {
    // Sleep for a bit then ask again.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    closing = waitpid(childId, &status, WNOHANG);
  }

  // We had an error instead of actually exiting succesfully.
  if (closing < 0) {
    std::cout << childId << '\n';
    throw std::runtime_error("Problem monitoring subthread.");
  }

  // We timed out, need to kill the subprocess.
  if (killVar.load()) {
    kill(childId, SIGKILL);
    closing = waitpid(childId, &status, 0);
  }

  // Set our return value and let the thread end.
  promise.set_value_at_thread_exit(static_cast<unsigned int>(status));
}

} // End anonymous namespace.


namespace tester {

Command::Command(const JSON &step, int64_t timeout) : timeout(timeout) {
  // Make sure the step has all of the values needed for construction.
  ensureContains(step, "stepName");
  ensureContains(step, "executablePath");
  ensureContains(step, "arguments");
  ensureContains(step, "output");

  // Build the command.
  name = step["stepName"];
  for (std::string arg : step["arguments"])
    args.push_back(arg);

  // Build the output.
  std::string outName = step["output"];

  // "-" represents stdout.
  if (outName == "-") {
#ifdef _WIN32
    throw std::runtime_error("Don't know how to capture stdout on Windows yet.");
#endif
    isStdOut = true;

    // Build a path in this directory for the standard output.
    fs::path fileName(name + "-temp.out");
    output = fs::current_path();
    output /= fileName;
  }
  // We've got a file name.
  else {
    isStdOut = false;

    // Make sure the output path is absolute.
    fs::path outPath(outName);
    if (outPath.is_absolute())
      output = outPath;
    else
      output = fs::absolute(outPath);
  }

  // Need to explicitly tell json what type we're pulling out here because it doesn't like loading
  // into an fs::path.
  std::string path = step["executablePath"];
  exePath = fs::path(path);
}

ExecutionOutput Command::execute(const ExecutionInput &ei) const {
  // Create our output context.
  ExecutionOutput eo(output);

  // Make the actual shell command, using our input and output contexts.
  std::vector<std::string> trueArgs;
  for (std::string arg : args)
    trueArgs.emplace_back(resolveArg(ei, eo, arg).string());
  std::string exe = resolveExe(ei, eo, exePath).string();
  std::string stdOutFile = isStdOut ? eo.getOutputFile().string() : "";

  // Create the futures for the thread.
  std::promise<unsigned int> promise;
  std::future<unsigned int> future = promise.get_future();

  // Create the kill condition.
  std::atomic_bool kill(false);

  // Run the command in another thread.
  std::thread thread = std::thread(
     runCommand,
     std::ref(promise), std::ref(kill), std::ref(exe), std::ref(trueArgs),std::ref(stdOutFile)
  );

  // Detach the thread to allow it to run in the background.
  thread.detach();

  // Wait to time out on the future. If we do, ask the thread to die.
  if (future.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout) {
    // Notify the thread we want it to die.
    kill.store(true);

    // Wait another timeout length on it to set the future. If it does then that means the thread
    // isn't dying probably because the subprocess isn't dying for some reason despite SIGKILL.
    // This return should be nearly instantaneous.
    if (future.wait_for(std::chrono::seconds(timeout)) != std::future_status::ready)
      throw std::runtime_error("Couldn't kill subprocess.");

    // We know we timed out, time to notify the higher-ups.
    throw TimeoutException("Subcommand timed out:\n  " + buildCommand(ei, eo));
  }

  // Finally get the result of the thread.
  int rv = future.get();

// If we're on a POSIX system then we need to decompose the return value appropriately.
#if __linux__ || __APPLE__
  // If we exited "normally" we need to check the return code. If the return code is 0, all is well.
  if (WIFEXITED(rv)) {
    // Get the exit status
    rv = WEXITSTATUS(rv);

    // If we the return code is not 0, we failed in some manner. Raise a custom exception.
    if (rv != 0)
      throw FailException("Subcommand returned status code " + std::to_string(rv)
                          + ":\n  " + buildCommand(ei, eo));
  }

  // If we exited due to a signal we can dump the signal and throw an exception.
  else if (WIFSIGNALED(rv)) {
    rv = WTERMSIG(rv);
    throw FailException("Subcommand terminated by signal " + std::to_string(rv)
                        + ":\n  " + buildCommand(ei, eo));
  }

  // We have some other status of the process. Throw a more generic error that will pass itself
  // all the way out of the program. This needs to be handled.
  else
    throw std::runtime_error("Subcommand terminated in an unknown fashion:\n  " + buildCommand(ei, eo));

// Best guess at decoding status code on Windows.
#elif _WIN32 || _WIN64
  LPDWORD status_code;
  bool success = GetExitCodeThread(handle, &status_code);
  if (!success)
    throw std::runtime_eror("Failed to get Windows process exit code.");

  if (status_code != 0)
    throw FailException("Subcommand returned status code " + std::to_string(rv)
                        + ":\n  " + command);
#else
  // We don't know how to get status on this platform... throw generic error.
  throw std::runtime_error("We don't know how to get status on this platform.")
#endif

  // Tell the toolchain about our output.
  return eo;
}

std::string Command::buildCommand(const ExecutionInput &ei, const ExecutionOutput &eo) const {
  // We start with the path to the exe.
  std::string command = resolveExe(ei, eo, exePath).string();

  // Then add new arguments, using the resolver to see if they're "magic" arguments.
  for (std::string arg : args) {
    command += ' ';
    command += resolveArg(ei, eo, arg).string();
  }

  // If we were initially writing to stdout, then we add the redirect note.
  if (isStdOut) {
#if __linux__ || __APPLE__
    command += " > \"" + eo.getOutputFile().string() + "\"";
#else
    command += " TO STDOUT";
#endif
  }

  return command;
}

fs::path Command::resolveArg(const ExecutionInput &ei, const ExecutionOutput &eo,
    std::string arg) const {
  // Input magic argument. Resolves to the input file for this command.
  if (arg == "$INPUT")
    return ei.getInputFile();

  // Output magic argument. Resolves to the output file for this command.
  if (arg == "$OUTPUT")
    return eo.getOutputFile();

  // Seem like it was meant to be a magic parameter.
  if (arg[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + arg);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(arg);
}

fs::path Command::resolveExe(const ExecutionInput &ei, const ExecutionOutput &eo,
                                std::string exe) const {
  // Exe magic argument. Resolves to the current "tested executable" (probably your compiler).
  if (exe == "$EXE")
    return ei.getTestedExecutable();

  // Input magic argument. Resolves to the input file for this command. Use for when a step
  // prodouces a runnable executable (your compiled executable).
  if (exe == "$INPUT")
    return ei.getInputFile();

  // Seem like it was meant to be a magic parameter.
  if (exe[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + exe);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(exe);
}

// Implement the Command ostream operator
std::ostream &operator<<(std::ostream &os, const Command &c) {
  ExecutionInput ei("$INPUT", "$EXE");
  ExecutionOutput eo(c.output);
  os << c.buildCommand(ei, eo);
  return os;
}

} // End namespace tester

