#include "testharness/TestHarness.h"

#include "toolchain/ExecutionState.h"
#include "toolchain/CommandException.h"

#include "util.h"

#include "dtl/dtl.hpp"

#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <utility>
#include <functional>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <exception>

// Convenience
namespace fs = std::experimental::filesystem;

// A private namespace to hold some file operations.
namespace {

// A vector of paths.
typedef std::vector<fs::path> PathList;

// Extensions of files we like. Declared as char[] so they can be used as template args.
constexpr char const inExt[] = ".in";
constexpr char const outExt[] = ".out";

// Filter that returns true if a path is a file the extension matches
template <const char *ext>
bool fileFilter(const fs::path &p) {
  if (!fs::is_regular_file(p))
    return false;
  return p.extension() == fs::path(ext);
}

bool directoryFilter(const fs::path &p) {
  return fs::is_directory(p) && ! fs::is_symlink(p);
}

// Function that gathers paths from a directory based on a supplied filter function
template<bool (*filter)(const fs::path &)>
void gatherFromDir(const fs::path &base, PathList &paths) {
for (const auto &it : fs::directory_iterator(base)) {
  fs::path f(it.path());
  if (filter(f))
    paths.push_back(f);
}
}

// Function that takes paths from two directores
template <void (*gatherIn)(const fs::path &, PathList &),
        void (*gatherOut)(const fs::path &, PathList &)>
void pairPaths(const fs::path &inDir, const fs::path &outDir, tester::PathList &pairs) {
  // Gather the paths from this directory.
  PathList in;
  PathList out;
  gatherIn(inDir, in);
  gatherOut(outDir, out);

  // Sort so we can do lexicographical comparison.
  std::sort(in.begin(), in.end());
  std::sort(out.begin(), out.end());

  // Match the paths and add to the list.
  auto inIt = in.begin(), inEnd = in.end();
  auto outIt = out.begin(), outEnd = out.end();
  while (inIt != inEnd && outIt != outEnd) {
    // We care about the stem, which is the filename without extension or the final directory in the
    // path.
    fs::path inStem = inIt->stem(), outStem = outIt->stem();
    if (inStem == outStem) {
      // Add the match.
      pairs.emplace_back(*inIt, *outIt);

      // Advance both iterators because we matched.
      ++inIt;
      ++outIt;
    }
    // No match, advance only one iterator. Check lexicographic ordering and advance the earlier.
    else if (inIt->filename() < outIt->filename())
      ++inIt;
    else
      ++outIt;
  }
}

// The templates are getting kind of long, time to shorten them. These are just pointers (aka
// aliases) to the created template functions for pairPaths. They have the same signature.
constexpr void(*getTests)(const fs::path &, const fs::path &, tester::PathList &) =
  &pairPaths<gatherFromDir<fileFilter<inExt>>, gatherFromDir<fileFilter<outExt>>>;
constexpr void(*getDirs)(const fs::path &, const fs::path &, tester::PathList &) =
  &pairPaths<gatherFromDir<directoryFilter>, gatherFromDir<directoryFilter>>;

void recurseFindTests(fs::path in, fs::path out, std::string prefix, tester::TestSet &tests) {
  // What's our current insert key? It's the current deepest directory (identical for in and out)
  // appended to the prefix.
  std::string key = prefix + in.stem().string();
  std::cout << "Got key: " << key << '\n';

  // Now pair them and insert them in the package for our key if there are any.
  tester::PathList testsHere;
  getTests(in, out, testsHere);
  if (!testsHere.empty())
    tests.insert(std::make_pair(key, testsHere));

  // Now recurse again.
  tester::PathList dirsHere;
  getDirs(in, out, dirsHere);
  for (tester::PathPair pair : dirsHere)
    recurseFindTests(pair.in, pair.out, key + ".", tests);
}

void findTests(fs::path in, fs::path out, tester::PackageSet &tests) {
  // Grab tests in each of the "packages" of tests.
  // First get the directories. These top level directories are "packages".
  tester::PathList dirsHere;
  getDirs(in, out, dirsHere);

  // Now iterate over the directories and find their matching tests. These become the packages of
  // tests, useful for competitive testing.
  for (tester::PathPair pair : dirsHere) {
    // The package name and its TestSet.
    std::string packName = pair.in.stem();
    tester::TestSet &packTests = tests[packName];

    // Find the tests.
    recurseFindTests(pair.in, pair.out, "", packTests);
  }
}

void getFileLines(fs::path fp, std::vector<std::string> &lines) {
  std::ifstream fs(fp);
  std::string buf;
  while (std::getline(fs, buf))
    lines.push_back(buf);
}

} // End anonymous namespace

// A private namespace to hold some test operations.
namespace {

tester::TestResult runTest(const tester::PathPair &tp, const tester::ToolChain &toolChain) {
  // Try to build the test. If there's a problem running a command, then we assume failure.
  fs::path output;
  try {
    tester::ExecutionOutput eo = toolChain.build(tp.in);
    output = eo.getOutputFile();
  }
  catch (const tester::CommandException &ce) {
    std::cout << "Command error: " << ce.what() << '\n';
    return tester::TestResult(tp.in, false, true, "");
  }

  // Get the lines from the files.
  std::vector<std::string> expLines;
  std::vector<std::string> genLines;
  getFileLines(tp.out, expLines);
  getFileLines(output, genLines);

  dtl::Diff<std::string> diff(expLines, genLines);
  diff.compose();
  diff.composeUnifiedHunks();

  // We failed the test.
  if (!diff.getUniHunks().empty()) {
    std::stringstream ss;
    diff.printUnifiedFormat(ss);
    return tester::TestResult(tp.in, false, false, ss.str());
  }

  return tester::TestResult(tp.in, true, false, "");
}

} // End anonymous namespace

namespace tester {

// Builds TestSet during object creation.
TestHarness::TestHarness(const JSON &json, bool quiet) : quiet(quiet) {
  // Make sure we have an executable to test then set it. Need to explicitly tell json what type
  // we're pulling out here because it doesn't like loading into an fs::path.
  ensureContains(json, "testedExecutablePaths");
  if (!json["testedExecutablePaths"].is_array())
    throw std::runtime_error("Tested executable paths was not an array.");

  for (const JSON &path : json["testedExecutablePaths"]) {
    if (!path.is_string())
      throw std::runtime_error("Executable path was not string.");

    std::string exe = path;
    testedExecutables.emplace_back(exe);
  }

  // Make sure toolchains are provided then build the set of toolchains.
  ensureContains(json, "toolchains");
  for (const JSON &toolchain : json["toolchains"])
    toolchains.emplace_back(toolchain);

  // Make sure an in and out dir were provided.
  ensureContains(json, "inDir");
  ensureContains(json, "outDir");

  // Get the in and out paths.
  std::string inDirStr = json["inDir"];
  std::string outDirStr = json["outDir"];
  fs::path inDir(inDirStr);
  fs::path outDir(outDirStr);

  // Ensure the paths exist.
  if (!fs::exists(inDir) || !fs::is_directory(inDir))
    throw std::runtime_error("Input file directory did not exist: " + inDirStr);
  if (!fs::exists(outDir) || !fs::is_directory(outDir))
    throw std::runtime_error("Output file directory did not exist: " + outDirStr);

  // Build the test set.
  findTests(inDir, outDir, tests);
}

void TestHarness::runTests() {
  for (fs::path exe : testedExecutables) {
    std::cout << "\nTesting executable: " << exe << '\n';
    for (ToolChain &tc : toolchains) {
      std::cout << "Running toolchain: " <<  tc.getBriefDescription() << '\n';
      tc.setTestedExecutable(exe);
      runTestsForToolChain(tc);
    }
  }
}

std::string TestHarness::getTestInfo() const {
  std::string rv = "Tests:\n";
  for (auto &tlEntry : tests) {
    rv += "  " + tlEntry.first + ": " + std::to_string(tlEntry.second.size()) + '\n';
  }
  return rv;
}

void TestHarness::runTestsForToolChain(const ToolChain &toolChain) {
  // Stat tracking for toolchain tests.
  unsigned int toolChainCount = 0, toolChainPasses = 0;

  // Iterate the packages.
  for (const auto &testPackage : tests) {
    // Print the package name.
    std::cout << "Entering package: " << testPackage.first << '\n';

    // Stat tracking for package tests.
    unsigned int packageCount = 0, packagePasses = 0;

    for (const auto &testSet : testPackage.second) {
      std::cout << "  Entering subpackage: " << testSet.first << '\n';

      // Track how many tests we run.
      packageCount += testSet.second.size();

      // Count how many passes we get.
      unsigned int subPackagePasses = 0;

      // Iterate over the tests.
      for (const PathPair &tp : testSet.second) {
        // Run the test and save the result.
        TestResult result = runTest(tp, toolChain);
        results.addResult(toolChain.getTestedExecutable(), testPackage.first, result);

        // Log the pass/fail.
        std::cout << "    " << tp.in.stem().string() << ": "
                  << (result.pass ? "PASS" : "FAIL") << '\n';

        // If we pass, note the pass.
        if (result.pass) {
          ++packagePasses;
          ++subPackagePasses;
        }
        // If we fail, potentially print the diff.
        else if (!quiet && !result.error)
          std::cout << '\n' << result.diff << '\n';
      }

      std::cout << "  Subpackage passed " << subPackagePasses << " / " << testSet.second.size()
                << '\n';
    }

    // Update the toolchain stats from the package stats.
    toolChainPasses += packagePasses;
    toolChainCount += packageCount;

    std::cout << " Package passed " << packagePasses<< " / " << packageCount << '\n';
  }

  std::cout << "Toolchain passed " << toolChainPasses << " / " << toolChainCount << "\n\n";
}

} // End namespace tester
