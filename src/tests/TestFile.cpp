#include "tests/TestFile.h"
#include "tests/TestParser.h"

namespace tester {

TestFile::TestFile(const fs::path& path) : usesInputStream(false), testPath(path) {
  
  fs::path tmpInsPath = fs::temp_directory_path() / testPath;

  TestParser parser(*this);

#if defined(DEBUG)
  std::cout << "tmpInsPath: " << tmpInsPath << std::endl;
#endif 

}

TestFile::~TestFile() {
  if (usesInputStream) {
    // fs::remove()
    std::cout << "Uses inputs stream... remove!" << std::endl;
  }
}

} // namespace tester