#include "tests/TestFile.h"
#include "tests/TestParser.h"

static uint64_t nextId = 0;

namespace {

std::string stripFileExtension(const std::string& str) {
  std::size_t lastIdx = str.find_last_of(".");
  return str.substr(0, lastIdx);
}

} // anonymous namespace

namespace tester {

// Initialize the static id to zero
std::atomic<uint64_t> TestFile::nextId(0);

// An atoimc 
uint64_t TestFile::generateId() {
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

TestFile::TestFile(const fs::path& path, const fs::path& artifactDir)
    : id(generateId()), testPath(path) {

  try {
    // Create .test-artifacts if it doesn't exist    
    if (!fs::exists(artifactDir)) {
      fs::create_directories(artifactDir);
    }

    // create .test-artifacts/testfiles if it doesn't exist
    fs::path testArtifactsDir = artifactDir / "testfiles"; 
    if (!fs::exists(testArtifactsDir)) {
      fs::create_directories(testArtifactsDir);
    }

    std::string testName = path.stem();
    fs::path basePath = testArtifactsDir / fs::path(testName + '-' + std::to_string(id));

    setInsPath(fs::path(basePath.string() + ".ins"));
    setOutPath(fs::path(basePath.string() + ".out"));

    // std::cout << "Creating file: " << testName << std::endl;
    // std::cout << "INS: " << getInsPath() << std::endl;  
    // std::cout << "OUT: " << getOutPath() << std::endl;  

  } catch (const fs::filesystem_error& e) {
    throw std::runtime_error("Filesystem error: " + std::string(e.what()));
  } 
}

TestFile::~TestFile() {

  // std::cout << "Calling Destructor...\n";
  try {
    if (fs::exists(insPath)) {
      // Remove temporary input stream file 
      fs::remove(insPath);
    }
    if (fs::exists(outPath)) {
      // Remove the tenmporary testfile directory and the expected out
      fs::remove(outPath);
    }
  } catch (const std::exception& e) {
    std::cerr << "Caught exception in destructor: "<< e.what() << std::endl;
  }
}

std::string TestFile::getParseErrorMsg() const {

  switch (getParseError()) {
    case ParseError::NoError:
      return "No error";
      break;
    case ParseError::DirectiveConflict:
      return "Two or more testfile directives supplied that can not "
             "coexist in one file.";
      break;
    case ParseError::FileError:
      return "A filepath provided in the testfile was unable to be "
             "located or opened.";
      break;
    case ParseError::RuntimeError:
      return "An unexpected runtime error occured while parsing the "
             "testifle.";
      break;
    default:
      return "No matching Parse Error";
  }
}

} // namespace tester