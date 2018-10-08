#include "analysis/spreadsheet/Cell.h"

#include "analysis/spreadsheet/Utility.h"

namespace tester {

void RefCell::dump(std::ostream &os) {
  os << '=' << posToCellName(cell.getCol(), cell.getRow());
}

void StringCell::dump(std::ostream &os) {
  os << str;
}

template <>
void IntCell<signed char>::dump(std::ostream &os) {
  os << static_cast<signed int>(value);
}

template <>
void IntCell<unsigned char>::dump(std::ostream &os) {
  os << static_cast<unsigned int>(value);
}

void RateCell::dump(std::ostream &os){
  os << '=' << value << " / " << posToCellName(maxCell.getCol(), maxCell.getRow());
}

void AverageCell::dump(std::ostream &os) {
  // Put beginning of sum.
  os << "=SUM(";

  // Iterate over cells to put in.
  for (auto it = cells.begin(); it != cells.end(); ++it) {
    const Cell &cell = it->get();
    os << posToCellName(cell.getCol(), cell.getRow());

    if ((it + 1) != cells.end())
      os << ',';
  }

  // Print end of sum and then the division.
  os << ") / " << cells.size();
}

void MultCell::dump(std::ostream &os) {
  os << '=' << posToCellName(cell.getCol(), cell.getRow()) << " * " << multiplier;
}

void CountIfsCell::dump(std::ostream &os) {
  // Put out begining of countifs.
  os << "=COUNTIFS(";

  // Iterate over conditions.
  for (auto it = conds.begin(); it != conds.end(); ++it) {
    const Cell &rMin = it->range.min, &rMax = it->range.max;
    os << posToCellName(rMin.getCol(), rMin.getRow()) << ':'
       << posToCellName(rMax.getCol(), rMax.getRow()) << ','
       << it->cond;

    if ((it + 1) != conds.end())
      os << ',';
  }

  os << ')';
}

} // End namespace tester
