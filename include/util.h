#ifndef TESTER_UTIL_H
#define TESTER_UTIL_H

#include "json.hpp"

#include <string>
#include <exception>

// Convenience.
using JSON = nlohmann::json;

namespace tester {

// Makes sure a JSON object contains a key. Throws an error if it does not.
inline void ensureContains(const JSON &json, std::string name) {
  if (json.count(name) == 0)
    throw std::runtime_error(name + " missing from JSON.");
}

}

#endif //TESTER_UTIL_H
