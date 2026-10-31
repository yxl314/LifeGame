#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "domain/board_limits.h"
#include "domain/boundary.h"
#include "domain/cell.h"
#include "domain/rule.h"

namespace lifegame::domain
{

/**
 * Fixed-capacity board backed by one contiguous std::array.
 *
 * The first width() * height() cells are active; the remaining storage is
 * always zero. A fixed capacity avoids allocation during simulation and lets
 * the parallel row slicer write into disjoint regions without locks.
 */
class Grid
{
public:
    static constexpr int kMaxWidth = kMaxBoardExtent;
    static constexpr int kMaxHeight = kMaxBoardExtent;
    static constexpr int kMaxCells = kMaxWidth * kMaxHeight;

    Grid() = default;
    explicit Grid(int width, int height);

    /**
     * Resize the active area.
     *
     * When the dimensions change the whole backing array is cleared, so the
     * unused region stays zero. When the dimensions are unchanged this is a
     * no-op: callers that overwrite every active cell (the step pipeline) avoid
     * a 160 KB memset per generation.
     */
    void resize(int width, int height);

    void clear();

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    bool inBounds(int row, int col) const;
    bool cell(int row, int col) const;
    void setCell(int row, int col, bool alive);

    /// Toggle a cell and return its new state. Out-of-bounds returns false
    /// (which is also the state after toggling an alive cell to dead).
    bool toggleCell(int row, int col);

    int population() const;
    int neighbourCount(int row, int col, Boundary boundary) const;

    /// Evolve one generation in place (serial; primarily for tests and single-step).
    void advance(const Rule& rule, Boundary boundary);

    void randomize(int densityPercent, std::uint32_t seed);

    bool equals(const Grid& other) const;

    const std::array<Cell, kMaxCells>& cells() const
    {
        return m_cells;
    }

    const Cell* data() const
    {
        return m_cells.data();
    }

    /**
     * Compute a whole generation into `next` (serial).
     *
     * `next` must not alias this object. Its dimension is adjusted before the
     * rows are computed.
     */
    static void computeNext(const Grid& current, Grid* next, const Rule& rule, Boundary boundary);

    /**
     * Compute a half-open row range into `next`.
     *
     * Different ranges only read `current` and only write their own rows of
     * `next`, so several tasks may run concurrently. `next` must already have
     * the same dimensions as `current`.
     */
    static void computeRows(const Grid& current,
                            Grid* next,
                            const Rule& rule,
                            Boundary boundary,
                            int rowBegin,
                            int rowEnd);

private:
    int m_width = 0;
    int m_height = 0;
    std::array<Cell, kMaxCells> m_cells{};
};

} // namespace lifegame::domain
