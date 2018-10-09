#ifndef TESTER_SHEET_H
#define TESTER_SHEET_H

#include "analysis/spreadsheet/Table.h"

#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace tester {

// A class that manages "tables" within a sheet. Sheets will eventually output a csv file that can
// be imported to an excel-like program. Table addition order matters when resolving cell
// references.
class Sheet {
public:
  // Adds a new table with a name to the sheet.
  template <typename T>
  T &addTable(const std::string &name, const std::string &title);

  // Get a table by its name.
  Table &getTable(const std::string &name) { return *tablesByName.at(name); }

  // Main output function. Dumps the sheet in SV format (separated like CSV but the separator is |).
  void dumpSV(std::ostream &os) const;

private:
  // Tables in this sheet. Manages the order they'll be output in.
  std::vector<std::string> orderedTables;

  // Tables by name.
  std::unordered_map<std::string, TablePtr> tablesByName;

  // Table titles
  std::unordered_map<std::string, std::string> tableTitles;

  static const char sepChar = '|';
};

} // End namespace tester

// Template implementation needs to be in header file.
template <typename T>
T &tester::Sheet::addTable(const std::string &name, const std::string &title) {
  // Make sure that T is a subclass of Table.
  static_assert(std::is_base_of<Table, T>::value, "Added table not inheritor of Table.");

  // Allocate the table. Hold the pointer here so we don't have to look up the table for its
  // reference when we put it in the map. We won't pass it away so the unique_ptr still owns it.
  auto *t = new T();
  tablesByName.emplace(std::make_pair(name, TablePtr(t)));

  // Put the name in the ordered list.
  orderedTables.push_back(name);

  // Save the title.
  tableTitles.emplace(std::make_pair(name, title));

  // Return the reference casted back to the base type.
  return static_cast<T&>(*t);
}

#endif //TESTER_SHEET_H
