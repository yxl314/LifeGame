#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "domain/board_limits.h"
#include "domain/boundary.h"
#include "domain/cell.h"
#include "domain/rule.h"

namespace lifegame::domain
{

/**
 * Heap-backed board with one contiguous active-cell vector.
 *
 * Storage is sized to the active width() * height() area only, so a small
 * board does not pay for a 1.5M-cell buffer. The vector is reused by the step
 * pipeline when dimensions are unchanged, and the Grid object itself stays
 * small enough to live on the stack. This is important for 1500x1000 boards.
 */
class Grid
{
public:
    static constexpr int kMaxWidth = kMaxBoardWidth;
    static constexpr int kMaxHeight = kMaxBoardHeight;
    static constexpr int kMaxCells = kMaxWidth * kMaxHeight;

    Grid();
    explicit Grid(int width, int height);

    /**
     * Resize the active area.
     *
     * When dimensions change the storage is sized to the new active area and
     * cleared. When dimensions and storage are unchanged this is a no-op:
     * callers that overwrite every active cell (the step pipeline) avoid a
     * large memset per generation.
     */
    void resize(int width, int height);

    void clear();

    constexpr int width() const noexcept
    {
        return m_width;
    }

    constexpr int height() const noexcept
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

    const std::vector<Cell>& cells() const
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
    std::vector<Cell> m_cells;
};

} // namespace lifegame::domain
