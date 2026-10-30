#pragma once

#include <vector>

namespace lifegame::controller
{

struct CellPosition
{
    int row = 0;
    int col = 0;
};

/// Bresenham line between two cells, inclusive. Pure C++ and unit-testable.
std::vector<CellPosition> rasterLine(int fromRow, int fromCol, int toRow, int toCol);

} // namespace lifegame::controller
