#ifndef DIRECTIVES_H
#define DIRECTIVES_H

#include <string>

namespace Directive {

// parsing directives from testfiles
inline const std::string INPUT = "INPUT:";
inline const std::string INPUT_FILE = "INPUT_FILE:";
inline const std::string CHECK = "CHECK:";
inline const std::string CHECK_FILE = "CHECK_FILE:";

// other constants
inline const uint32_t MAX_INPUT_BYTES = 4096;
inline const uint32_t MAX_OUTPUT_BYTES = 4096;

} // namespace Directive

#endif // DIRECTIVES_H