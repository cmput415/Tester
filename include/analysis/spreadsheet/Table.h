#ifndef TESTER_TABLE_H
#define TESTER_TABLE_H

#include "analysis/spreadsheet/Cell.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace tester {

// Typedefs making referencing tables easier.
class Table;
typedef std::unique_ptr<Table> TablePtr;

typedef std::vector<std::vector<CellPtr>> Cells;

// A class that manages a single table in a sheet.
class Table {
public:
  // Make class virtual with virtual default destructor.
  virtual ~Table() = default;

  const Cells &getCells() const { return cells; }

protected:
  // Default constructor is restricted to children.
  Table() = default;

protected:
  // Our internal cells.
  Cells cells;
};

// A class managing the count of tests in this analysis.
class TestCountTable : public Table {
public:
  // Construct the cells with two empty vectors.
  TestCountTable() { for (int i = 0; i < 2; ++i) cells.emplace_back(); }

  // Add a test count.
  void addTestCount(std::string name, size_t count);

  // Get the test count cell for a name.
  const Cell &getTestCount(const std::string &name) { return *cells[1][colByName.at(name)]; }

private:
  std::unordered_map<std::string, size_t> colByName;
};

// An (n+1)x(n+1) table where the first row and column are named as students and the rest are left
// available for anaylsis to fill.
class CrossTable : public Table {
public:
  // Initialise isReserved to false.
  CrossTable() : Table(), isReserved(false) { }

  // Reserve spots for students.
  void reserve(const std::vector<std::string> &students);

  // Gets the cell crossing between a defender and an attacker.
  const Cell &getCrossCell(const std::string &defender, const std::string &attacker);

protected:
  // Is this already reserved.
  bool isReserved;

  // Identify a cell by a student name.
  std::unordered_map<std::string, size_t> idxByName;
};

// Uses a crosstable to hold per-toolchain pass rates.
class TestPassRateTable : public CrossTable {
public:
  // Add a pass rate to the table.
  void addPassRate(const std::string &defender, const std::string &attacker, size_t passCount,
                   const Cell &maxCount);
};

// Uses a crosstable to hold per-participant, averaged pass rates.
class TestSummaryTable : public CrossTable {
public:
  // Add a pass rate to the table.
  void addSummary(const std::string &defender, const std::string &attacker,
                  const std::vector<CellRef> &cells);
};

} // End namespace tester

#endif //TESTER_TABLE_H
