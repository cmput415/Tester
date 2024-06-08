#include "TestFile.h"
#include "TestResult.h"
#include "config/Config.h"

namespace tester {

TestResult runTest(TestFile* test, const ToolChain& toolChain,
                   const Config& cfg);

} // namespace tester
