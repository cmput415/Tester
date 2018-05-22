#include "TestHarness.h"

#include <map>
#include <algorithm>
#include <exception>

#define TESTER_TESTER_DEBUG 1

namespace {
// A vector of paths.
typedef std::vector<fs::path> PathList;

// Extensions of files we like.
const fs::path inExt(".in");
const fs::path outExt(".out");

// Gathers all files in a directory with a certain extension.
void gatherDirFiles(fs::path base, fs::path extFilter, PathList &paths) {
  for (const auto &it : fs::directory_iterator(base)) {
    // Skip directories.
    if (fs::is_directory(it))
      continue;

    fs::path f(it.path());

    // Skip bad extensions.
    if (f.extension() != extFilter)
      continue;

    // Save the path.
    paths.push_back(f);
  }
}

tester::TestList pairFiles(fs::path in, fs::path out) {
  // A place to hold our paths.
  PathList inFiles;
  PathList outFiles;

  // Get the files out of the directory.
  gatherDirFiles(in, inExt, inFiles);
  gatherDirFiles(out, outExt, outFiles);

  // Sort so we can do lexicographical comparison.
  std::sort(inFiles.begin(), inFiles.end());
  std::sort(outFiles.begin(), outFiles.end());

  // Try to pair up input and output files.
  tester::TestList matched;
  auto inIt = inFiles.begin(), inEnd = inFiles.end();
  auto outIt = outFiles.begin(), outEnd = outFiles.end();
  while (inIt != inEnd && outIt != outEnd) {
    // We care about the stem, which is the filename without extension.
    fs::path inStem = inIt->stem(), outStem = outIt->stem();
    if (inStem == outStem) {
      // Add the match.
      matched.emplace_back(*inIt, *outIt);

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

  return matched;
}

} // End anonymous namespace

namespace tester {

// Builds TestSet during object creation.
Tester::Tester(const JSON &json) : toolchain(json) {
  std::string inDirStr = json["inDir"];
  std::string outDirStr = json["outDir"];

  fs::path inDir(inDirStr);
  fs::path outDir(outDirStr);

  if (!fs::exists(inDir))
    throw std::runtime_error("Input file directory did not exist: " + inDirStr);

  if (!fs::exists(outDir))
    throw std::runtime_error("Output file directory did not exist: " + outDirStr);

  // Get our base directory tests. Ideally this is empty, but you never know.
  tests.emplace(".", pairFiles(inDir, outDir));
}


} // End namespace tester
