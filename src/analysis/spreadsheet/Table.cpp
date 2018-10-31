#include "analysis/spreadsheet/Table.h"

#include "analysis/spreadsheet/Utility.h"

#include <cassert>

namespace tester {

void MapTable::addCell(const std::string &name, CellPtr cell) {
  // Sanity checks.
  assert(cells.size() == 2 && "More than two rows in map table.");
  assert(colByName.count(name) == 0 && "Adding col that exists already in map table.");

  // Add this column to the name register.
  colByName.emplace(std::make_pair(name, cells[0].size()));

  // Add the name and the count.
  cells[0].emplace_back(new StringCell(name));
  cells[1].emplace_back(std::move(cell));
}

void CrossTable::reserve(const std::vector<std::string> &students) {
  // Sanity check.
  assert(!isReserved && "Reserving a cross table twice.");

  // Reserve the space. Adding one for the name column on top.
  size_t dim = students.size();
  cells.reserve(dim + 1);

  // Fill the space. Adding one for the name column on the left.
  for (size_t i = 0; i < dim + 1; ++i)
    cells.emplace_back(dim + 1);

  // Now fill the slots with names while building student to index map.
  for (size_t i = 0; i < students.size(); ++i) {
    // Get the index into the table, it's the same for row/col. One extra to offset the extra name
    // row/col.
    size_t idx = i + 1;

    // Save the name and index in the map.
    const std::string &name = students[i];
    idxByName.emplace(std::make_pair(name, idx));

    // Put the name in the defender row and attacker column. Using reset to replace what's there
    // already.
    cells[0][idx].reset(new StringCell(name));
    cells[idx][0].reset(new StringCell(name));
  }

  // Put a string into the top left corner.
  cells[0][0].reset(new StringCell("defender\\attacker"));

  // Mark as already reserved.
  isReserved = true;
}

const Cell &CrossTable::getCrossCell(const std::string &defender, const std::string &attacker) {
  // Sanity check.
  assert(isReserved && "Getting cell from unreserved cross table.");
  CellPtr &cell = cells[idxByName.at(defender)][idxByName.at(attacker)];
  assert(cell && "Accessing cross cell that isn't yet filled.");

  // Return the cell.
  return *cell;
}

void CrossTable::addCrossCell(const std::string &defender, const std::string &attacker,
                              CellPtr toAdd) {
  // Sanity checks.
  assert(isReserved && "Adding cell to unreserved cross table.");
  CellPtr &cell = cells[idxByName.at(defender)][idxByName.at(attacker)];
  assert(!cell && "Filling cross table cell that is already filled.");

  // The actual add.
  cell = std::move(toAdd);
};

CellRange CrossTable::getDefenderNameRange() {
  CellPtr &min = cells[1][0];
  CellPtr &max = cells[idxByName.size()][0];
  return {*min, *max};
}

CellRange CrossTable::getAttackerNameRange() {
  CellPtr &min = cells[0][1];
  CellPtr &max = cells[0][idxByName.size()];
  return {*min, *max};
}

CellRange CrossTable::getDefenderRange(const std::string &name) {
  size_t idx = idxByName.at(name);
  CellPtr &min = cells[idx][1];
  CellPtr &max = cells[idx][idxByName.size()];
  return {*min, *max};
}

CellRange CrossTable::getAttackerRange(const std::string &name) {
  size_t idx = idxByName.at(name);
  CellPtr &min = cells[1][idx];
  CellPtr &max = cells[idxByName.size()][idx];
  return {*min, *max};
}

SummaryTable::SummaryTable(std::vector<std::pair<std::string, std::string>> categories) : Table() {
  cells.reserve(categories.size() + 2ul);

  // Add names row.
  CellVec names;
  names.emplace_back(new StringCell("Names"));
  cells.emplace_back(std::move(names));

  // Add row for each category.
  for (const auto &category : categories) {
    // Track row number.
    rowByName.emplace(std::make_pair(category.first, cells.size()));

    // Add row.
    CellVec catVec;
    catVec.emplace_back(new StringCell(category.second));
    cells.emplace_back(std::move(catVec));
  }

  // Add the summary row.
  CellVec summary;
  summary.emplace_back(new StringCell("Summary"));
  cells.emplace_back(std::move(summary));
}

const Cell& SummaryTable::getSummary(const std::string &name)  {
  return *cells[cells.size() - 1][colByName.at(name)];
}

CellRange SummaryTable::getSummaryRange() {
  const CellVec &lastRow = cells[cells.size() - 1];
  return {*lastRow[1], *lastRow[lastRow.size() - 1]};
}

void TestCountTable::addTestCount(const std::string &name, size_t count) {
  addCell(name, CellPtr(new IntCell<size_t>(count)));
}

void OffensivePointsTable::addAttacker(const std::string &name, CellRange pointRange,
                                       CellRange nameRange) {
  // Create the cell and add conditions to it.
  auto *cell = new SumIfsCell(pointRange);
  // Note that there's no guard against score here. The thing we don't want to sum is when the
  // defender passed everything... but then their fail rate is zero, so summing does nothing.
  cell->addCondition(nameRange, ConditionPtr(new LiteralCondition<std::string>("<>", name)));
  // Note that there's no guard against solution here. You get bonus points for breaking solution.

  addCell(name, CellPtr(cell));
};

void DefensivePointsTable::addDefender(const std::string &name, CellRange pointRange,
                                       CellRange nameRange){
  // Create the cell and add conditions to it.
  auto *cell = new CountIfsCell();
  cell->addCondition(pointRange, ConditionPtr(new LiteralCondition<int>("=", 1)));
  cell->addCondition(nameRange, ConditionPtr(new LiteralCondition<std::string>("<>", name)));
  cell->addCondition(nameRange, ConditionPtr(new LiteralCondition<std::string>("<>", "solution")));

  addCell(name, CellPtr(cell));
};

void CoveragePointsTable::addCoverage(const std::string &name, const Cell &coverage,
                                      CellRange pointRange, CellRange nameRange) {
  auto *cell = new MultIfsCell(coverage);
  cell->addCondition(pointRange, ConditionPtr(new LiteralCondition<int>("<", 1)));
  cell->addCondition(nameRange, ConditionPtr(new LiteralCondition<std::string>("<>", name)));
  // Note that there's no guard against solution here. You get bonus points for breaking solution.

  addCell(name, CellPtr(cell));
}

void ToolchainPassRateTable::addPassRate(const std::string &defender, const std::string &attacker,
                                         size_t passCount, const Cell &maxCount) {
  addCrossCell(defender, attacker, CellPtr(new RateCell(passCount, maxCount)));
}

void TotalPassRateTable::addPassRate(const std::string &defender, const std::string &attacker,
                                     const std::vector<CellRef> &addends) {
  addCrossCell(defender, attacker, CellPtr(new AverageCell(addends)));
}

void TotalFailRateTable::addFailRate(const std::string &defender, const std::string &attacker,
                                     const tester::Cell &passRate) {
  addCrossCell(defender, attacker, CellPtr(new PercentComplementCell(passRate)));
}

PointSummaryTable::PointSummaryTable()
    : SummaryTable({{"offense", "Offense"}, {"defense", "Defense"}, {"self", "Self Testing"},
                    {"method", "Test Methodology"}}) { }

void PointSummaryTable::addSummary(const std::string &name, const Cell &offense,
                                   const Cell &defense, const Cell &self, const Cell &method) {
  // Sanity check.
  size_t size = cells[0].size();
  for (const auto &row : cells)
    assert(row.size() == size && "Summary table row length unequal.");

  // Track index.
  colByName.emplace(std::make_pair(name, size));
  cells[0].emplace_back(new StringCell(name));

  // Add sections for offense and defense. Hold onto the pointer so we can make the summary cell.
  Cell *offenseCell = new RefCell(offense);
  Cell *defenseCell = new MultCell<int>(defense, 2);
  cells[rowByName.at("offense")].emplace_back(offenseCell);
  cells[rowByName.at("defense")].emplace_back(defenseCell);

  // Add section for self testing.
  ConditionPtr cond(new CellCondition("=", self, 1));
  Cell *selfCell = new IfCell(std::move(cond), 1, 0);
  cells[rowByName.at("self")].emplace_back(selfCell);

  // Add section for testing method.
  Cell *methodCell = new RefCell(method);
  cells[rowByName.at("method")].emplace_back(methodCell);

  // Add summary cell.
  std::vector<CellRef> sumCells{*offenseCell, *defenseCell, *selfCell, *methodCell};
  cells[cells.size() - 1].emplace_back(new SumCell(std::move(sumCells)));
}

FinalSummaryTable::FinalSummaryTable()
    : SummaryTable({{"style", "Grammar and Code Style (15%)"},
                    {"abide","Functionality + Guideline Abiding (15%)"},
                    {"passRate", "Teaching-Team Tests (50%)"},
                    {"tournament", "Competitive Testing tournament (20%)"}}) { }

void FinalSummaryTable::addSummary(const std::string &name, const Cell &points,
                                   const CellRange &pointsRange, const Cell &passRate) {
  // Sanity check.
  size_t size = cells[0].size();
  for (const auto &row : cells)
    assert(row.size() == size && "Summary table row length unequal.");

  // Track index.
  colByName.emplace(std::make_pair(name, size));
  cells[0].emplace_back(new StringCell(name));

  // Add style and abide sections. Hold onto the pointer so we can make the summary cell.
  Cell *styleCell = new StringCell("FILL ME (MAX .15)");
  Cell *abideCell = new StringCell("FILL ME (MAX .15)");
  cells[rowByName.at("style")].emplace_back(styleCell);
  cells[rowByName.at("abide")].emplace_back(abideCell);

  // Add teaching team pass rate section.
  Cell *passRateCell = new MultCell<float>(passRate, .5f);
  cells[rowByName.at("passRate")].emplace_back(passRateCell);

  // Add tournament score cell.
  Cell *tournamentCell = new TournamentResultsCell(pointsRange, points, .2f);
  cells[rowByName.at("tournament")].emplace_back(tournamentCell);

  // Add summary cell.
  std::vector<CellRef> sumCells{*styleCell, *abideCell, *passRateCell, *tournamentCell};
  cells[cells.size() - 1].emplace_back(new SumCell(std::move(sumCells)));
}

} // End namespace tester
