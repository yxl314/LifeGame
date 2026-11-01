#pragma once

/**
 * @file line_raster.h
 * @brief Bresenham line rasterization helper.
 */

#include <vector>

namespace lifegame::controller
{

/// @brief Integer cell coordinate.
struct CellPosition
{
    int row = 0; ///< Row index.
    int col = 0; ///< Column index.
};

/**
 * @brief Rasterize a Bresenham line between two cells, inclusive.
 * @param fromRow Start row.
 * @param fromCol Start column.
 * @param toRow End row.
 * @param toCol End column.
 * @return Ordered cell positions along the line.
 */
std::vector<CellPosition> rasterLine(int fromRow, int fromCol, int toRow, int toCol);

} // namespace lifegame::controller
