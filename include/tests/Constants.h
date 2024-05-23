#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace Constants {

  // parsing directives from testfiles  
  inline const std::string INPUT_DIRECTIVE = "INPUT:";
  inline const std::string INPUT_FILE_DIRECTIVE = "INPUT_FILE:";
  inline const std::string CHECK_DIRECTIVE = "CHECK:";
  inline const std::string CHECK_FILE_DIRECTIVE = "CHECK_FILE:";

  // other constants
  inline const uint32_t MAX_INPUT_BYTES = 1025;
}

#endif // CONSTANTS_H
