#include "domain/pattern.h"

#include <algorithm>
#include <utility>

namespace lifegame::domain
{

bool Pattern::alive(int row, int col) const
{
    if (row < 0 || row >= height || col < 0 || col >= width)
    {
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(row) * static_cast<std::size_t>(width) +
                              static_cast<std::size_t>(col);
    return index < cells.size() && cells[index] != kDead;
}

bool Pattern::isConsistent() const
{
    if (width < 0 || height < 0)
    {
        return false;
    }
    return cells.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
}

int Pattern::population() const
{
    int count = 0;
    for (const Cell value : cells)
    {
        count += value != kDead ? 1 : 0;
    }
    return count;
}

Pattern Pattern::fromAscii(std::string id, const std::vector<std::string>& rows)
{
    Pattern pattern;
    pattern.id = std::move(id);
    pattern.height = static_cast<int>(rows.size());

    std::size_t widest = 0;
    for (const std::string& row : rows)
    {
        widest = std::max(widest, row.size());
    }
    pattern.width = static_cast<int>(widest);
    pattern.cells.assign(static_cast<std::size_t>(pattern.width) *
                             static_cast<std::size_t>(pattern.height),
                         kDead);

    for (int row = 0; row < pattern.height; ++row)
    {
        const std::string& text = rows[static_cast<std::size_t>(row)];
        for (std::size_t col = 0; col < text.size(); ++col)
        {
            const char ch = text[col];
            const bool alive = ch == 'O' || ch == 'o' || ch == '#' || ch == 'X' || ch == 'x';
            pattern.cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(pattern.width) +
                          col] = alive ? kAlive : kDead;
        }
    }

    return pattern;
}

} // namespace lifegame::domain
