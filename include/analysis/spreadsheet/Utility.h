#ifndef TESTER_UTILITY_H
#define TESTER_UTILITY_H

#include <ostream>

namespace tester {

class Condition {
public:
  // Dump the condition.
  virtual void dump(std::ostream &os) = 0;
};

std::string idxToColName(size_t idx);

std::string posToCellName(size_t col, size_t row);



}

#endif //TESTER_UTILITY_H
