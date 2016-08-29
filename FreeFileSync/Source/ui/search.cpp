// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "search.h"
#include <zen/zstring.h>
#include <zen/perf.h>

using namespace zen;


namespace
{
template <bool respectCase>
class MatchFound
{
public:
    MatchFound(const std::wstring& textToFind) : textToFind_(textToFind) {}
    bool operator()(const std::wstring& phrase) const { return contains(phrase, textToFind_); }

private:
    const std::wstring textToFind_;
};


template <>
class MatchFound<false>
{
public:
    MatchFound(const std::wstring& textToFind) : textToFind_(makeUpperCopy(textToFind)) {}
    bool operator()(std::wstring&& phrase) const { return contains(makeUpperCopy(phrase), textToFind_); }

private:
    const std::wstring textToFind_;
};

//###########################################################################################

template <bool respectCase>
ptrdiff_t findRow(const Grid& grid, //return -1 if no matching row found
                  const std::wstring& searchString,
                  bool searchAscending,
                  size_t rowFirst, //specify area to search:
                  size_t rowLast)  // [rowFirst, rowLast)
{
    if (auto prov = grid.getDataProvider())
    {
        std::vector<Grid::ColumnAttribute> colAttr = grid.getColumnConfig();
        erase_if(colAttr, [](const Grid::ColumnAttribute& ca) { return !ca.visible_; });
        if (!colAttr.empty())
        {
            const MatchFound<respectCase> matchFound(searchString);

            if (searchAscending)
            {
                for (size_t row = rowFirst; row < rowLast; ++row)
                    for (const Grid::ColumnAttribute& ca : colAttr)
                        if (matchFound(prov->getValue(row, ca.type_)))
                            return row;
            }
            else
                for (size_t row = rowLast; row-- > rowFirst;)
                    for (const Grid::ColumnAttribute& ca : colAttr)
                        if (matchFound(prov->getValue(row, ca.type_)))
                            return row;
        }
    }
    return -1;
}
}


std::pair<const Grid*, ptrdiff_t> zen::findGridMatch(const Grid& grid1, const Grid& grid2, const std::wstring& searchString, bool respectCase, bool searchAscending)
{
    //PERF_START

    const size_t rowCount1 = grid1.getRowCount();
    const size_t rowCount2 = grid2.getRowCount();

    size_t cursorRow1 = grid1.getGridCursor();
    if (cursorRow1 >= rowCount1)
        cursorRow1 = 0;

    std::pair<const Grid*, ptrdiff_t> result(nullptr, -1);

    auto finishSearch = [&](const Grid& grid, size_t rowFirst, size_t rowLast)
    {
        const ptrdiff_t targetRow = respectCase ?
                                    findRow<true >(grid, searchString, searchAscending, rowFirst, rowLast) :
                                    findRow<false>(grid, searchString, searchAscending, rowFirst, rowLast);
        if (targetRow >= 0)
        {
            result = std::make_pair(&grid, targetRow);
            return true;
        }
        return false;
    };

    if (searchAscending)
    {
        if (!finishSearch(grid1, cursorRow1 + 1, rowCount1))
            if (!finishSearch(grid2, 0, rowCount2))
                finishSearch(grid1, 0, cursorRow1 + 1);
    }
    else
    {
        if (!finishSearch(grid1, 0, cursorRow1))
            if (!finishSearch(grid2, 0, rowCount2))
                finishSearch(grid1, cursorRow1, rowCount1);
    }
    return result;
}