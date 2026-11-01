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
 * Fields are private: callers use accessors and the constructor/factory. The
 * id is stable but carries no display text (names belong to the view).
 */
class Pattern
{
public:
    Pattern() = default;
    Pattern(std::string id, int width, int height, std::vector<Cell> cells);

    const std::string& id() const noexcept
    {
        return m_id;
    }

    int width() const noexcept
    {
        return m_width;
    }

    int height() const noexcept
    {
        return m_height;
    }

    const std::vector<Cell>& cells() const noexcept
    {
        return m_cells;
    }

    bool alive(int row, int col) const;
    int population() const;

    /// True when width/height are non-negative and cells matches width * height.
    bool isConsistent() const;

    /**
     * Build a pattern from ASCII art.
     *
     * 'O', 'o', '#', 'X' and 'x' are alive; every other character is dead.
     * Rows may have different lengths and are padded on the right.
     */
    static Pattern fromAscii(std::string id, const std::vector<std::string>& rows);

private:
    std::string m_id;
    int m_width = 0;
    int m_height = 0;
    std::vector<Cell> m_cells;
};

} // namespace lifegame::domain
