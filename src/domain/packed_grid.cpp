#include "domain/packed_grid.h"

#include "domain/grid.h"

#include <algorithm>

namespace lifegame::domain
{

PackedGrid::PackedGrid(const Grid& board)
    : m_width(board.width()), m_height(board.height())
{
    const std::size_t count =
        static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
    m_words.assign((count + 63u) / 64u, 0);

    const Cell* cells = board.data();
    for (std::size_t i = 0; i < count; ++i)
    {
        if (cells[i] != kDead)
        {
            m_words[i >> 6] |= (std::uint64_t(1) << (i & 63u));
        }
    }
}

Grid PackedGrid::unpack() const
{
    Grid board(m_width, m_height);
    if (m_words.empty())
    {
        return board;
    }

    const int width = m_width;
    const int height = m_height;
    for (int row = 0; row < height; ++row)
    {
        const std::size_t rowStart = static_cast<std::size_t>(row) * static_cast<std::size_t>(width);
        for (int col = 0; col < width; ++col)
        {
            const std::size_t index = rowStart + static_cast<std::size_t>(col);
            if ((m_words[index >> 6] & (std::uint64_t(1) << (index & 63u))) != 0)
            {
                board.setCell(row, col, true);
            }
        }
    }
    return board;
}

} // namespace lifegame::domain
