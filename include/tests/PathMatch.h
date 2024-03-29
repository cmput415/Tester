#ifndef TESTER_PATH_MATCH_H
#define TESTER_PATH_MATCH_H

#include <filesystem>
#include <cassert>

#include <iostream>

// Convenience.
namespace fs = std::filesystem;

namespace tester{

// Struct that functions as a holding place for a set of matched paths.
struct PathMatch {
  // No default constructor.
  PathMatch() = delete;

  // Construct with in path, out path, and an instream path that may be empty.
  PathMatch(fs::path in, fs::path out, fs::path inStream)
      : in(std::move(in)), out(std::move(out)), inStream(std::move(inStream)) {
    assert(!this->in.empty() && "Input file was empty.");
    assert(!this->out.empty() && "Output file was empty.");
  }

  // The matched files..
  const fs::path in, out, inStream;
};

} // End namespace tester
#endif //TESTER_PATH_MATCH_H
