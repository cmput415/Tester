#include "analysis/spreadsheet/Sheet.h"

#include <utility>

namespace tester {

void Sheet::dumpSV(std::ostream &os) const {
  // Track row number outside tables. Sheets start counting at 1.
  size_t rowCount = 1;
  for (const std::string &name : orderedTables) {
    // Write title.
    os << tableTitles.at(name) << '\n';
    ++rowCount;

    // Get table and iterate over cells.
    const Table &table = *tablesByName.at(name);
    for (const auto &row : table.getCells()) {
      // Track the column position here.
      size_t colCount = 0;
      for (auto it = row.begin(); it != row.end(); ++it) {
        // Set the cell's position and dump it. (Increment column number.)
        (*it)->setPosition(colCount++, rowCount);
        (*it)->dump(os);

        // Add the separator char if we're not the end of the row.
        if ((it + 1) != row.end())
          os << sepChar;
      }

      // Done a row, go to next row.
      os << '\n';
      ++rowCount;
    }

    // Skip a row between tables.
    os << '\n';
    ++rowCount;
  }
}

} // End namespace tester
