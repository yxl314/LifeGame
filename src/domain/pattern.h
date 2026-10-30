#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "domain/cell.h"

namespace lifegame::domain
{

/**
 * A rectangular block of cells.
 *
 * The pattern carries a stable `id` but no display text: names, categories and
 * descriptions are presentation concerns owned by the view layer.
 */
struct Pattern
{
    std::string id;
    int width = 0;
    int height = 0;
    std::vector<Cell> cells;

    bool alive(int row, int col) const;
    int population() const;

    /**
     * Build a pattern from ASCII art.
     *
     * 'O', 'o', '#', 'X' and 'x' are alive; every other character is dead.
     * Rows may have different lengths and are padded on the right.
     */
    static Pattern fromAscii(std::string id, const std::vector<std::string>& rows);
};

} // namespace lifegame::domain
