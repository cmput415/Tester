#ifndef COLORS_H
#define COLORS_H

#include <string>

namespace Colors {

inline const std::string GREEN = "\033[32m", RED = "\033[31m",
                         YELLOW = "\033[33m", BLUE = "\033[34m",
                         MAGENTA = "\033[35m", CYAN = "\033[36m",
                         WHITE = "\033[37m", BLACK = "\033[30m",
                         BG_WHITE = "\033[47m", RESET = "\033[0m";
}

#endif