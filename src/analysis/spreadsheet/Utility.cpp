#include "analysis/spreadsheet/Utility.h"

#include "analysis/spreadsheet/Cell.h"

namespace tester {

void CellCondition::dump(std::ostream &os) {
  os << posToCellName(cell.getCol(), cell.getRow()) << op << value;
}

std::string idxToColName(size_t idx) {
  std::string result;

  // If we're on the second round of the alphabet, then we need to insert the second character.
  if (idx > 25)
    result += static_cast<char>(65 + idx / 26 - 1);

  result += static_cast<char>(65 + idx % 26);

  return result;
}

std::string posToCellName(size_t col, size_t row) {
  return idxToColName(col) + std::to_string(row);
}

}
