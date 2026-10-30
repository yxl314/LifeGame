#include "domain/grid.h"

#include <algorithm>
#include <cassert>
#include <random>
#include <utility>

namespace lifegame::domain
{

Grid::Grid(int width, int height)
{
    resize(width, height);
}

void Grid::resize(int width, int height)
{
    const int newWidth = std::clamp(width, 0, kMaxWidth);
    const int newHeight = std::clamp(height, 0, kMaxHeight);
    if (newWidth == m_width && newHeight == m_height)
    {
        return;
    }

    m_width = newWidth;
    m_height = newHeight;
    m_cells.fill(kDead);
}

void Grid::clear()
{
    m_cells.fill(kDead);
}

bool Grid::inBounds(int row, int col) const
{
    return row >= 0 && row < m_height && col >= 0 && col < m_width;
}

bool Grid::cell(int row, int col) const
{
    if (!inBounds(row, col))
    {
        return false;
    }
    return m_cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(m_width) +
                   static_cast<std::size_t>(col)] != kDead;
}

void Grid::setCell(int row, int col, bool alive)
{
    if (!inBounds(row, col))
    {
        return;
    }
    m_cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(m_width) +
            static_cast<std::size_t>(col)] = alive ? kAlive : kDead;
}

bool Grid::toggleCell(int row, int col)
{
    if (!inBounds(row, col))
    {
        return false;
    }

    Cell& value = m_cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(m_width) +
                         static_cast<std::size_t>(col)];
    value = value != kDead ? kDead : kAlive;
    return value != kDead;
}

int Grid::population() const
{
    const int count = m_width * m_height;
    int alive = 0;
    for (int i = 0; i < count; ++i)
    {
        alive += m_cells[static_cast<std::size_t>(i)] != kDead ? 1 : 0;
    }
    return alive;
}

int Grid::neighbourCount(int row, int col, Boundary boundary) const
{
    if (!inBounds(row, col) || m_width == 0 || m_height == 0)
    {
        return 0;
    }

    int neighbours = 0;
    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0)
            {
                continue;
            }

            int r = row + dr;
            int c = col + dc;
            if (boundary == Boundary::Wrap)
            {
                r = ((r % m_height) + m_height) % m_height;
                c = ((c % m_width) + m_width) % m_width;
            }
            else if (!inBounds(r, c))
            {
                continue;
            }

            neighbours += m_cells[static_cast<std::size_t>(r) * static_cast<std::size_t>(m_width) +
                                  static_cast<std::size_t>(c)] != kDead
                              ? 1
                              : 0;
        }
    }
    return neighbours;
}

void Grid::advance(const Rule& rule, Boundary boundary)
{
    if (m_width <= 0 || m_height <= 0)
    {
        return;
    }

    Grid next;
    computeNext(*this, &next, rule, boundary);
    *this = std::move(next);
}

void Grid::randomize(int densityPercent, std::uint32_t seed)
{
    const int density = std::clamp(densityPercent, 0, 100);
    std::mt19937 engine(seed);
    std::uniform_int_distribution<int> distribution(1, 100);

    const int count = m_width * m_height;
    for (int i = 0; i < count; ++i)
    {
        m_cells[static_cast<std::size_t>(i)] = distribution(engine) <= density ? kAlive : kDead;
    }
}

bool Grid::equals(const Grid& other) const
{
    if (m_width != other.m_width || m_height != other.m_height)
    {
        return false;
    }

    const int count = m_width * m_height;
    return std::equal(m_cells.begin(),
                      m_cells.begin() + static_cast<std::size_t>(count),
                      other.m_cells.begin());
}

void Grid::computeNext(const Grid& current, Grid* next, const Rule& rule, Boundary boundary)
{
    if (next == nullptr)
    {
        return;
    }

    const int width = current.width();
    const int height = current.height();
    if (width <= 0 || height <= 0)
    {
        next->resize(0, 0);
        return;
    }

    next->resize(width, height);
    computeRows(current, next, rule, boundary, 0, height);
}

void Grid::computeRows(const Grid& current,
                       Grid* next,
                       const Rule& rule,
                       Boundary boundary,
                       int rowBegin,
                       int rowEnd)
{
    if (next == nullptr)
    {
        return;
    }

    const int width = current.width();
    const int height = current.height();
    if (width <= 0 || height <= 0 || next->width() != width || next->height() != height)
    {
        return;
    }

    const int begin = std::max(0, rowBegin);
    const int end = std::min(height, rowEnd);
    if (begin >= end)
    {
        return;
    }

    assert(next != &current);

    const Cell* source = current.data();
    Cell* destination = next->data();

    for (int row = begin; row < end; ++row)
    {
        const int rowOffset = row * width;
        const bool hasTop = row > 0;
        const bool hasBottom = row + 1 < height;

        for (int col = 0; col < width; ++col)
        {
            int neighbours = 0;

            // The common case: interior cell, no boundary check at all.
            if (hasTop && hasBottom && col > 0 && col + 1 < width)
            {
                const int index = rowOffset + col;
                neighbours = source[index - width - 1] + source[index - width] +
                             source[index - width + 1] + source[index - 1] + source[index + 1] +
                             source[index + width - 1] + source[index + width] +
                             source[index + width + 1];
            }
            else if (boundary == Boundary::Dead)
            {
                for (int dr = -1; dr <= 1; ++dr)
                {
                    const int r = row + dr;
                    if (r < 0 || r >= height)
                    {
                        continue;
                    }
                    for (int dc = -1; dc <= 1; ++dc)
                    {
                        if (dr == 0 && dc == 0)
                        {
                            continue;
                        }
                        const int c = col + dc;
                        if (c < 0 || c >= width)
                        {
                            continue;
                        }
                        neighbours += source[r * width + c] != kDead ? 1 : 0;
                    }
                }
            }
            else
            {
                for (int dr = -1; dr <= 1; ++dr)
                {
                    const int r = ((row + dr) % height + height) % height;
                    for (int dc = -1; dc <= 1; ++dc)
                    {
                        if (dr == 0 && dc == 0)
                        {
                            continue;
                        }
                        const int c = ((col + dc) % width + width) % width;
                        neighbours += source[r * width + c] != kDead ? 1 : 0;
                    }
                }
            }

            const bool alive = source[rowOffset + col] != kDead;
            destination[rowOffset + col] =
                static_cast<Cell>((alive ? rule.survives(neighbours) : rule.births(neighbours)) ? kAlive
                                                                                               : kDead);
        }
    }
}

} // namespace lifegame::domain
