#include "analysis/spreadsheet/Utility.h"

#include "analysis/spreadsheet/Cell.h"

namespace tester {

void IntLiteralCondition::dump(std::ostream &os) {
  os << '"' << op << value << '"';
}

void CellCondition::dump(std::ostream &os) {
  os << posToCellName(cell.getCol(), cell.getRow()) << '"' << op << value << '"';
}

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

}
