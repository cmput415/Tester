#include "toolchain/Command.h"

#include "util.h"

#include "toolchain/CommandException.h"

#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <thread>
#include <unistd.h>

#if __linux__
#include <wait.h>
#elif __APPLE__
#include <signal.h>
#include <sys/wait.h>
#endif

namespace {

/// @brief Open the file with provided flags and mode. Redirect the file descriptor
/// supplied by dup_fd to the file underlying file_str. 
int redirectOpen(const std::string& file_str, int flags, mode_t mode, int dup_fd) {

  // Open the process
  int fd = open(file_str.c_str(), flags, mode);
  if (fd == -1) {
    return -1;
  }

  // Set the file descriptor aliased by dup_fd to the newly opened file 
  if (dup2(fd, dup_fd) == -1) {
    close(fd);
    return -1;
  }
  close(fd);
  return 0; 
}

void becomeCommand(const std::string& exe,
                   const std::vector<std::string>& trueArgs,
                   const std::string& input,
                   const std::string& output,
                   const std::string& error,
                   const std::string& runtime
  ) {
  // Error and output files should never be empty.
  assert(!error.empty() && !output.empty());

  // Build a list of true arguments.
  const char* args[trueArgs.size() + 2];

  // Build the args list
  args[0] = exe.c_str();
  for (std::size_t i = 0; i < trueArgs.size(); ++i)
    args[i + 1] = trueArgs[i].c_str();
  args[trueArgs.size() + 1] = NULL;

  // Build the new command's environment (PATH, LD_PRELOAD).
  std::string path = "PATH=" + std::string(std::getenv("PATH"));

  #if __linux__
    std::string preload = "LD_PRELOAD=" + runtime;
  #elif __APPLE__
    std::string preload = "DYLD_INSERT_LIBRARIES=" + runtime;
  #endif

  std::string ld_library_path = "LD_LIBRARY_PATH=" + fs::path(runtime).parent_path().string();

  // Construct the environment variables array.
  const char* env[4];
  env[0] = path.c_str();
  env[1] = !runtime.empty() ? preload.c_str() : NULL;
  env[2] = ld_library_path.c_str();
  env[3] = NULL;

  // Open the supplied files and redirect FD of the current child process to them.
  int outFileStatus = redirectOpen(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, STDOUT_FILENO);
  int errorFileStatus = redirectOpen(error.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, STDERR_FILENO);
  int inFileStatus = !input.empty() ? redirectOpen(input.c_str(), O_RDONLY, 0, STDIN_FILENO) : 0;

  // If opening any of the supplied output, input, or error files failed, raise here.
  if (outFileStatus == -1 || errorFileStatus == -1 || inFileStatus == -1) {
      perror("dup2");
      exit(EXIT_FAILURE);
  }

  // Replace ourselves with the command.
  execve(exe.c_str(), const_cast<char* const*>(args), const_cast<char* const*>(env));

  // If execve returns, an error occurred.
  perror("execve");
  exit(EXIT_FAILURE);
}

// This can get a bit complicated. We want easy command running which is
// available in a cross platform manner via std::system but there's no way for
// us to kill a long running subprocess (i.e. there's an infinite loop in a
// test). This means we need to fall back on forking/execing, unfortunately.
void runCommand(std::promise<unsigned int>& promise, std::atomic_bool& killVar,
                const std::string& exe, const std::vector<std::string>& trueArgs,
                const std::string& input,
                const std::string& output,
                const std::string& error,
                const std::string& runtime) {

  pid_t childId = fork();

  // We're the child process, we want to replace our process image with the
  // shell running the command. This function will never return if successful
  // and will throw a runtime_error if it is unsuccessful.
  if (childId == 0)
    becomeCommand(exe, trueArgs, input, output, error, runtime);

  // We're in the parent process. Set up variables for watching the child
  // process.
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
    perror("waitpid,WNOHANG");
    throw std::runtime_error("Problem monitoring subprocess.");
  }

  // We didn't stop monitoring because of an error, so we have two options:
  // successful exit or timeout. If we timed out, we need to kill the
  // subprocess. Also check if closing is already set. This would mean that
  // the above loop stopped because we exited successfully, but in the
  // meantime there was a timeout (this race condition is impossible to
  // remove, but we can handle it). This means that the child has already been
  // reaped so we should not kill and wait on it. We check for equality with
  // zero because < 0 is handled above and > 0 we have already killed.
  if (killVar.load() && closing == 0) {
    // Try to kill the sub process.
    int killResult = kill(childId, SIGKILL);

    // Somehow we weren't able to send a kill signal to our child process.
    if (killResult < 0) {
      perror("kill");
      throw std::runtime_error("Problem killing subprocess. Check for zombie processes.");
    }

    // Try to wait on the killed subprocess to reap it.
    closing = waitpid(childId, &status, 0);

    // We had an error instead of killing successfully. Very weird
    // considering we send SIGKILL not SIGTERM.
    if (closing < 0) {
      perror("waitpid,0");
      throw std::runtime_error("Problem waiting on killed subprocess. "
                               "Check for zombie processes.");
    }
  }
  // Set our return value and let the thread end.
  promise.set_value_at_thread_exit(static_cast<unsigned int>(status));
}

} // End anonymous namespace.

namespace tester {

Command::Command(const JSON& step, int64_t timeout)
    : usesRuntime(false), usesInStr(false), timeout(timeout) {
  // Make sure the step has all of the values needed for construction.
  ensureContains(step, "stepName");
  ensureContains(step, "executablePath");
  ensureContains(step, "arguments");

  // Build the command.
  name = step["stepName"];
  for (std::string arg : step["arguments"])
    args.push_back(arg);

  // If no output path is supplied by default, temporaries are created to capture stdout and stderr.
  std::string output_name = std::string(step["stepName"]) + ".stdout";
  std::string error_name = std::string(step["stepName"]) + ".stderr";
  outPath = fs::temp_directory_path() / output_name;
  errPath = fs::temp_directory_path() / error_name;

  // Set the executable path
  std::string path = step["executablePath"];
  exePath = fs::path(path);

  // Allow override of stdout path
  if (doesContain(step, "output"))
    outputFile = fs::path(step["output"]);

  // Do we use an input stream file?
  if (doesContain(step, "usesInStr"))
    usesInStr = step["usesInStr"];

  // Do we use a runtime?
  if (doesContain(step, "usesRuntime"))
    usesRuntime = step["usesRuntime"];

  // Do we allow errors?
  if (doesContain(step, "allowError"))
    allowError = step["allowError"];
}

ExecutionOutput Command::execute(const ExecutionInput& ei) const {
  // Create our output context.
  fs::path out = outputFile.has_value() ? *outputFile : outPath;
  ExecutionOutput eo(out, errPath);

  // Always remove old output files so we know if a new one was created
  std::error_code ec;

  // Get the exe and its arguments, the things used in the actual execution of
  // the command.
  std::string exe = resolveExe(ei, eo, exePath).string();
  std::vector<std::string> trueArgs;
  for (const std::string& arg : args)
    trueArgs.emplace_back(resolveArg(ei, eo, arg).string());

  // Get the runtime path and standard out file, the things used in setting up
  // the execution of the command.
  std::string runtimeStr = usesRuntime ? ei.getTestedRuntime().string() : "";
  std::string inPathStr = usesInStr ? ei.getInputStreamFile().string() : "";
  std::string outPathStr = outPath.string();
  std::string errPathStr = errPath.string();

  // Create the promise, which gives the future for the thread, and the kill
  // variable, the things used in the monitor thread.
  std::promise<unsigned int> promise;
  std::future<unsigned int> future = promise.get_future();
  std::atomic_bool kill(false);

  // Run the command in another thread.
  auto start = std::chrono::high_resolution_clock::now(); // start recording timings
  std::thread thread =
      std::thread(runCommand, std::ref(promise), std::ref(kill), // Parent variables.
                  std::ref(exe), std::ref(trueArgs),             // Child execution variables.
                  std::ref(inPathStr),
                  std::ref(outPathStr),
                  std::ref(errPathStr), 
                  std::ref(runtimeStr));

  // Detach the thread to allow it to run in the background.
  thread.detach();

  // Wait to time out on the future. If we do, ask the thread to die.
  if (future.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout) {
    // Notify the thread we want it to die.
    kill.store(true);

    // Wait another timeout length on it to set the future. If it does then
    // that means the thread isn't dying probably because the subprocess
    // isn't dying for some reason despite SIGKILL. This return should be
    // nearly instantaneous.
    if (future.wait_for(std::chrono::seconds(timeout)) != std::future_status::ready)
      throw std::runtime_error("Couldn't kill subprocess.");

    // We know we timed out, time to notify the higher-ups.
    throw TimeoutException("Subcommand timed out:\n  " + buildCommand(ei, eo));
  }

  // Finally get the result of the thread.
  int rv = future.get();
  auto end = std::chrono::high_resolution_clock::now();

  // If we exited "normally" we need to check the return code. If the return
  // code is 0, all is well.
  if (WIFEXITED(rv)) {
    // Get the exit status
    rv = WEXITSTATUS(rv);

    // If the return code is not 0 and we do not allow errors, we failed in
    // some manner. Raise a custom exception.
    if (rv != 0 && !allowError)
      throw FailException("Subcommand returned status code " + std::to_string(rv) + ":\n  " +
                          buildCommand(ei, eo));
  }

  // If we exited due to a signal we can dump the signal and throw an
  // exception.
  else if (WIFSIGNALED(rv)) {
    rv = WTERMSIG(rv);
    throw FailException("Subcommand terminated by signal " + std::to_string(rv) + ":\n  " +
                        buildCommand(ei, eo));
  }

  // We have some other status of the process. Throw a more generic error that
  // will pass itself all the way out of the program. This needs to be
  // handled.
  else
    throw std::runtime_error("Subcommand terminated in an unknown fashion:\n  " +
                             buildCommand(ei, eo));

  // Tell the toolchain about our output.
  std::chrono::duration<double> elapsed = end - start;
  eo.setElapsedTime(elapsed.count());
  eo.setReturnValue(rv);
  return eo;
}

std::string Command::buildCommand(const ExecutionInput& ei, const ExecutionOutput& eo) const {
  // We start with the path to the exe.
  std::string command = resolveExe(ei, eo, exePath).string();

  // Then add new arguments, using the resolver to see if they're "magic"
  // arguments.
  for (const std::string& arg : args) {
    command += ' ';
    command += resolveArg(ei, eo, arg).string();
  }

  return command;
}

/**
 * @brief Replaces the placeholder in the original string with the contents
 * of to_replace.
 * @param original The original string containing the placeholder.
 * @param placeholder The placeholder to be replaced.
 * @param to_replace The string to replace the placeholder with.
 * @return A new string with the placeholder replaced by to_replace.
 */
std::string fillArgPlaceholder(const std::string& original, const std::string& placeholder,
                               const std::string& to_replace) {
  std::string result = original;
  std::size_t pos = result.find(placeholder);
  if (pos != std::string::npos) {
    result.replace(pos, placeholder.length(), to_replace);
  }
  return result;
}

/**
 * @brief Strips the .so extension and lib prefix from the given library filename.
 * @param filename The name of the dynamic library file.
 * @return The name of the library with the .so extension and lib prefix stripped.
 */
std::string stripLibraryName(const std::string& filename) {
  std::string result = filename;
  if (result.substr(0, 3) == "lib") {
    result = result.substr(3);
  }
  std::size_t pos = result.find(".so");
  if (pos != std::string::npos) {
    result = result.substr(0, pos);
  }
  return result;
}

fs::path Command::resolveArg(const ExecutionInput& ei, const ExecutionOutput& eo,
                             std::string arg) const {
  // Input magic argument. Resolves to the input file for this command.
  if (arg == "$INPUT")
    return ei.getInputFile();

  // Output magic argument. Resolves to the output file for this command.
  if (arg == "$OUTPUT")
    return eo.getOutputFile();

  // Two additional magic parameters for using the llc toolchain, which requires providing
  // the runtime path and the library name.
  std::string rtPath = "$RT_PATH";
  std::string rtLib = "$RT_LIB";

  // Set up placeholder replacements
  fs::path current_rt = ei.getTestedRuntime();
  std::string runtime_file = current_rt.filename().string();
  std::string runtime_dir = current_rt.parent_path().string();
  if (arg.find(rtPath) != std::string::npos) {
    // insert the directory of the runtime into the arg
    arg = fillArgPlaceholder(arg, rtPath, runtime_dir);
  }
  if (arg.find(rtLib) != std::string::npos) {
    // insert the name of the runtime library (for example gazrt from libgazrt.so) into the arg.
    arg = fillArgPlaceholder(arg, rtLib, stripLibraryName(runtime_file));
  }
  // Seem like it was meant to be a magic parameter.
  if (arg[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + arg);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(arg);
}

fs::path Command::resolveExe(const ExecutionInput& ei, const ExecutionOutput& eo,
                             std::string exe) const {
  // Exe magic argument. Resolves to the current "tested executable" (probably
  // your compiler).
  if (exe == "$EXE")
    return ei.getTestedExecutable();

  // Input magic argument. Resolves to the input file for this command. Use
  // for when a step prodocues a runnable executable (your compiled
  // executable).
  if (exe == "$INPUT")
    return ei.getInputFile();

  // Seem like it was meant to be a magic parameter.
  if (exe[0] == '$')
    throw std::runtime_error("Should this be a different magic paramter: " + exe);

  // Wasn't a special arg, we should just return the arg.
  return fs::path(exe);
}

// Implement the Command ostream operator
std::ostream& operator<<(std::ostream& os, const Command& c) {
  ExecutionInput ei("$INPUT", "", "$EXE", "");
  ExecutionOutput eo(c.outPath, c.errPath);
  os << c.buildCommand(ei, eo);
  return os;
}

} // End namespace tester
