#include "analysis/spreadsheet/Utility.h"


std::string idxToColName(size_t idx) {
  std::string result;

  if (idx < 26)
    result += static_cast<char>(65 + idx);
  else {
    result += static_cast<char>(65 + idx / 26);
    result += static_cast<char>(65 + idx % 26);
  }
  return result;
}

std::string posToCellName(size_t col, size_t row) {
  return idxToColName(col) + std::to_string(row);
}
