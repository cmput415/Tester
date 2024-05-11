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
constexpr char const inStrExt[] = ".ins";

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
      paths.emplace_back(std::move(f));
  }
}

// Function that takes paths from two directores
template <
    void (*gatherIn)(const fs::path &, PathList &),
    void (*gatherOut)(const fs::path &, PathList &),
    void (*gatherInStr)(const fs::path &, PathList &)
>
void matchPaths(const fs::path &inDir, const fs::path &outDir, const fs::path &inStrDir,
                tester::PathList &pairs) {
  // Gather the paths from this directory.
  PathList in;
  PathList out;
  gatherIn(inDir, in);
  gatherOut(outDir, out);

  // Gather in stream things if we were given a path.
  PathList inStr;
  if (!inStrDir.empty())
    gatherInStr(inStrDir, inStr);

  // Sort so we can do lexicographical comparison.
  std::sort(in.begin(), in.end());
  std::sort(out.begin(), out.end());
  std::sort(inStr.begin(), inStr.end());

  // Match the paths and add to the list.
  auto inIt = in.begin(), inEnd = in.end();
  auto outIt = out.begin(), outEnd = out.end();
  auto inStrIt = inStr.begin(), inStrEnd = inStr.end();
  while (inIt != inEnd && outIt != outEnd) {
    // We care about the stem, which is the filename without extension or the final directory in the
    // path.
    fs::path inStem = inIt->stem(), outStem = outIt->stem();
    if (inStem == outStem) {
      // We need to check if there's an in stream file match. That means that the in stream file
      // stem also matches the input file stem. Because this is optional we might not find one. We
      // first advance the iterator until it either hits the end or the filename is no longer
      // lexicographically less than the input file stem.
      while (inStrIt != inStrEnd && inStrIt->stem() < inStem)
        ++inStrIt;

      // Add the match. We need to know if there was an in stream file match as well. We use boolean
      // short circuiting to make sure we don't evaluate an end iterator's fields. If there's a
      // match add it, else just send empty string.
      if (inStrIt != inStrEnd && inStrIt->stem() == inStem)
        pairs.emplace_back(*inIt, *outIt, *inStrIt);
      else
        pairs.emplace_back(*inIt, *outIt, "");

      // Advance both iterators because we matched.
      ++inIt;
      ++outIt;
    }
      // No match, advance only one iterator. Check lexicographic ordering and advance the earlier.
    else if (inStem < outStem)
      ++inIt;
    else
      ++outIt;
  }
}

// The templates are getting kind of long, time to shorten them. These are just pointers (aka
// aliases) to the created template functions for matchPaths. They have the same signature.
constexpr
void(*getTests)(const fs::path &, const fs::path &, const fs::path &, tester::PathList &) =
  &matchPaths<
      gatherFromDir<fileFilter<inExt>>,
      gatherFromDir<fileFilter<outExt>>,
      gatherFromDir<fileFilter<inStrExt>>
  >;
constexpr
void(*getDirs)(const fs::path &, const fs::path &, const fs::path &, tester::PathList &) =
  &matchPaths<
      gatherFromDir<directoryFilter>,
      gatherFromDir<directoryFilter>,
      gatherFromDir<directoryFilter>
  >;

void recurseFindTests(const fs::path &in, const fs::path &out, const fs::path &inStr,
                      const std::string &prefix, tester::TestSet &tests) {
  // What's our current insert key? It's the current deepest directory (identical for in and out)
  // appended to the prefix.
  std::string key = prefix + in.stem().string();

  // Now pair them and insert them in the package for our key if there are any.
  tester::PathList testsHere;
  getTests(in, out, inStr, testsHere);
  if (!testsHere.empty())
    tests.insert(std::make_pair(key, testsHere));

  // Now recurse again.
  tester::PathList dirsHere;
  getDirs(in, out, inStr, dirsHere);
  for (tester::PathMatch match : dirsHere)
    recurseFindTests(match.in, match.out, match.inStream, key + ".", tests);
}

} // End anonymous namespace

// Namespace that holds our actual exported functions.
namespace tester {

void findTests(fs::path in, fs::path out, fs::path inStream, tester::PackageSet &tests) {
  // Grab tests in each of the "packages" of tests.
  // First get the directories. These top level directories are "packages".
  tester::PathList dirsHere;
  getDirs(in, out, inStream, dirsHere);

  // Now iterate over the directories and find their matching tests. These become the packages of
  // tests, useful for competitive testing.
  for (tester::PathMatch match : dirsHere) {
    // The package name and its TestSet.
    std::string packName = match.in.stem().string();
    tester::TestSet packTests;

    // Find the tests.
    recurseFindTests(match.in, match.out, match.inStream, "", packTests);

    // If we actually found any tests, add it to the list.
    if (packTests.size() > 0)
      tests.emplace(std::make_pair(packName, std::move(packTests)));
  }
}

bool hasFiles(const fs::path& path) {
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    if (fs::is_regular_file(entry.status())) {
      return true;
    }
  }
  return false;
}

std::vector<PathMatch> fillSubpackage(fs::path subPackagePath) {
  std::vector<PathMatch> testFiles; 
  for (const auto& testFile : fs::directory_iterator(subPackagePath)) {
    if (fs::is_regular_file(testFile)) {
      std::cout << "REGULAR FILE:" << testFile.path().filename() << std::endl;
      testFiles.push_back(PathMatch(testFile));
    }
  }
  return testFiles;
}

void findSubpackages(const fs::path &searchPath, std::vector<tester::SubPackage>& subPackages) {
  try {
    for (const auto& subDir : fs::directory_iterator(searchPath)) {
      if (fs::is_directory(subDir)) {
        std::cout << "SUBPACKAGE: " << subDir.path().filename() << std::endl; 
        if (hasFiles(subDir)) {
          tester::SubPackage subPackage;
          subPackage.emplace(subDir.path(), fillSubpackage(subDir.path()));
          subPackages.push_back(subPackage);
        }
        findSubpackages(subDir, subPackages);
      }
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void findTests(fs::path testsPath, tester::Module &module) {
  
  for (const auto& dir: fs::directory_iterator(testsPath)) {
    std::cout << "PACKAGE: " << dir.path().filename() << std::endl;

    tester::Package package;  
    std::vector<tester::SubPackage> subPackages;

    // fill subpackages 
    findSubpackages(dir.path(), subPackages);
    package[dir.path()] = subPackages;

    // insert package into packageSet
    module[dir.path().filename()] = package;
  }

  for (const auto& package : module) {
    std::cout << "MODULE COUNT " << std::endl;
  }
}

} // End namespace tests
