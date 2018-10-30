#ifndef TESTER_CELL_H
#define TESTER_CELL_H

#include "analysis/spreadsheet/Utility.h"

#include <cassert>
#include <ostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace tester {

// Typedefs making referencing cells easier.
class Cell;
typedef std::unique_ptr<Cell> CellPtr;
typedef std::reference_wrapper<const Cell> CellRef;

// A class that manages a single cell in a table.
class Cell {
public:
  // Default constructor fills position with zeroes.
  Cell() : isSet(false), col(0), row(0) { }

  // Make class virtual with virtual default destructor.
  virtual ~Cell() = default;

  // Set the cell position.
  void setPosition(size_t col_, size_t row_) {
    assert(!isSet && "Setting cell position twice.");
    col = col_; row = row_; isSet = true;
  }

  // Get the cell Col.
  size_t getCol() const {
    assert(isSet && "Getting cell column without setting.");
    return col;
  }

  // Get the cell Row.
  size_t getRow() const {
    assert(isSet && "Getting cell row without setting.");
    return row;
  }

  // Dump the cell contents to a stream.
  virtual void dump(std::ostream &os) = 0;

private:
  // Has been set to position.
  bool isSet;

  // The cells position.
  size_t col, row;
};

// A cell that takes the value of another cell.
class RefCell : public Cell {
public:
  // No default constructor.
  RefCell() = delete;

  // Construct with the cell to reference.
  explicit RefCell(const Cell &cell) : cell(cell) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The cell to reference.
  const Cell &cell;
};

// A class that represents a string in a cell.
class StringCell : public Cell {
public:
  // No default constructor.
  StringCell() = delete;

  // Construct with internal string.
  explicit StringCell(std::string str_) : Cell(), str(std::move(str_)) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The internal string.
  std::string str;
};

// A class that represents an integral value in a cell.
template <typename T>
class IntCell : public Cell {
public:
  // No default constructor.
  IntCell() = delete;

  // Construct with internal value.
  explicit IntCell(T value) : Cell(), value(value) {
    static_assert(std::is_integral<T>::value, "IntCell type is not integral.");
  }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The internal value.
  T value;
};

// A class that represents a sum of other cells.
class SumCell : public Cell {
public:
  // No default constructor.
  SumCell() = delete;

  // Construct with internal value.
  explicit SumCell(std::vector<CellRef> cells) : Cell(), cells(std::move(cells)) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // Cells to sum.
  const std::vector<CellRef> cells;
};

// A class that represents a "rate" in a cell. That is an integral divided by the value in another
// cell.
class RateCell : public Cell {
public:
  // No default constructor.
  RateCell() = delete;

  // Construct with value and the cell containing the max.
  RateCell(size_t value, const Cell &maxCell)  : Cell(), value(value), maxCell(maxCell) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The actual count.
  size_t value;

  // The max count cell.
  const Cell &maxCell;
};

// A class that represents an average of cells (sum / count).
class AverageCell : public Cell {
public:
  // No default constructor.
  AverageCell() = delete;

  // Construct with cells to average over.
  explicit AverageCell(std::vector<CellRef> cells) : Cell(), cells(std::move(cells)) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // Cells to average over.
  const std::vector<CellRef> cells;
};

// A cell that mulitplies the value of another cell by a constant.
template <typename T>
class MultCell : public Cell {
public:
  // No default constructor.
  MultCell() = delete;

  // Construct with the cell to reference.
  MultCell(const Cell &cell, T multiplier) : cell(cell), multiplier(multiplier) {
    static_assert(std::is_arithmetic<T>::value, "Mult cell not using a number type.");
  }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // Cell to multiply.
  const Cell &cell;

  // Multiplier
  const T multiplier;
};

// A cell that takes one value or another based on a condition (a ternary).
class IfCell : public Cell {
public:
  // No default constructor.
  IfCell() = delete;

  // Construct with condition and two values.
  IfCell(ConditionPtr cond, int trueVal, int falseVal)
      : cond(std::move(cond)), trueVal(trueVal), falseVal(falseVal) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The condition.
  ConditionPtr cond;

  // The true and false values.
  int trueVal, falseVal;
};

class CountIfsCell : public Cell {
public:
  // Only default constructor. Add conditions after.
  CountIfsCell() = default;

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

  // Add a condition to a the if.
  void addCondition(CellRange range, ConditionPtr cond) {
    conds.emplace_back(range, std::move(cond));
  }

private:
  // Our conditions.
  std::vector<RangeCondition> conds;
};

// A specialised cell for calculating tournament results.
class TournamentResultsCell : public Cell {
public:
  // No default constructor.
  TournamentResultsCell() = delete;

  // Construct with range of tournament results, a specific tournament result, and a scaling factor
  // (how much it's worth).
  TournamentResultsCell(CellRange results, const Cell &cell, float scale)
      : results(results), cell(cell), scale(scale) { }

  // Dump the cell contents to a stream.
  void dump(std::ostream &os) override;

private:
  // The tournament results.
  CellRange results;

  // The specific results.
  const Cell &cell;

  // The scaling factor.
  float scale;
};

} // End namespace tester

// Template implementation needs to be in header file.
// Specialise on char.
template <>
void tester::IntCell<signed char>::dump(std::ostream &os);
template <>
void tester::IntCell<unsigned char>::dump(std::ostream &os);

template <typename T>
void tester::IntCell<T>::dump(std::ostream &os) {
  os << value;
}

template <typename T>
void tester::MultCell<T>::dump(std::ostream &os) {
  os << '=' << tester::posToCellName(cell.getCol(), cell.getRow()) << " * " << multiplier;
}

#endif //TESTER_CELL_H
