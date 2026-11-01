#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace lifegame::domain
{

class Grid;

/**
 * Bit-packed, immutable snapshot of a board.
 *
 * Only history snapshots use this form: the live board and the simulation
 * pipeline stay byte-per-cell for cache-friendly neighbour reads. Packing is
 * done once per snapshot; unpacking happens only when a rewind starts.
 */
class PackedGrid
{
public:
    PackedGrid() = default;
    explicit PackedGrid(const Grid& board);

    Grid unpack() const;

    int width() const noexcept
    {
        return m_width;
    }

    int height() const noexcept
    {
        return m_height;
    }

    std::size_t bitSize() const noexcept
    {
        return static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
    }

    std::size_t byteSize() const noexcept
    {
        return m_words.size() * sizeof(std::uint64_t);
    }

    bool empty() const noexcept
    {
        return m_words.empty();
    }

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint64_t> m_words;
};

} // namespace lifegame::domain
