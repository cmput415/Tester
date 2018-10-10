#include "tests/testUtil.h"

#include <algorithm>

// Private namespace that holds utility functions for the functions that are actually exported. You
// can find the actual functions at the bottom of the file.
namespace {

// A vector of paths.
typedef std::vector<fs::path> PathList;

// Extensions of files we like. Declared as char[] so they can be used as template args.
constexpr char const inExt[] = ".in";
constexpr char const outExt[] = ".out";

// Filter that returns true if a path is a file the extension matches
template <const char *ext>
inline bool fileFilter(const fs::path &p) {
  if (!fs::is_regular_file(p))
    return false;
  return p.extension() == fs::path(ext);
}

inline bool directoryFilter(const fs::path &p) {
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

} // End anonymous namespace

// Namespace that holds our actual exported functions.
namespace tester {

void findTests(fs::path in, fs::path out, tester::PackageSet &tests) {
  // Grab tests in each of the "packages" of tests.
  // First get the directories. These top level directories are "packages".
  tester::PathList dirsHere;
  getDirs(in, out, dirsHere);

  // Now iterate over the directories and find their matching tests. These become the packages of
  // tests, useful for competitive testing.
  for (tester::PathPair pair : dirsHere) {
    // The package name and its TestSet.
    std::string packName = pair.in.stem().string();
    tester::TestSet packTests;

    // Find the tests.
    recurseFindTests(pair.in, pair.out, "", packTests);

    // If we actually found any tests, add it to the list.
    if (packTests.size() > 0)
      tests.emplace(std::make_pair(packName, std::move(packTests)));
  }
}

} // End namespace tests
