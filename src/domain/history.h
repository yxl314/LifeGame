#pragma once

#include <memory>
#include <vector>

#include "domain/grid.h"

namespace lifegame::domain
{

/**
 * Bounded snapshot history used to implement rewind.
 *
 * Life is many-to-one, so the previous generation cannot be derived from the
 * current one. We keep one snapshot per interval (plus an anchor at the
 * origin) and replay forward from the closest snapshot not newer than the
 * requested generation.
 *
 * This class is pure C++ and has no Qt dependency.
 */
class History
{
public:
    struct Snapshot
    {
        int generation = 0;
        std::shared_ptr<const Grid> board;
    };

    static constexpr int kMaxSnapshots = 64;

    explicit History(int interval = 100);

    /// Discard everything and start a new epoch at `generation`.
    void reset(int generation, std::shared_ptr<const Grid> board);

    /**
     * Record a snapshot when `generation` is aligned with the epoch and is
     * newer than the last stored snapshot. Older/duplicate generations are
     * ignored.
     */
    void record(int generation, std::shared_ptr<const Grid> board);

    /// True when `record(generation, ...)` would actually store something.
    bool shouldRecord(int generation) const;

    /// Remove every snapshot at or after `generation` (used after edits/rewind).
    void invalidateFrom(int generation);

    int interval() const;

    /// Generation at which the current epoch (anchor) started.
    int epochGeneration() const;
    void setInterval(int interval); // does not reset; the model resets explicitly

    int oldestGeneration() const;

    /// Closest snapshot whose generation is <= the requested one, or nullptr.
    const Snapshot* snapshotAtOrBefore(int generation) const;

    const std::vector<Snapshot>& snapshots() const
    {
        return m_snapshots;
    }

    bool empty() const
    {
        return m_snapshots.empty();
    }

private:
    bool aligned(int generation) const;
    void trim();

    int m_interval = 100;
    int m_epoch = 0;
    std::vector<Snapshot> m_snapshots;
};

} // namespace lifegame::domain
