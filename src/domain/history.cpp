#include "domain/history.h"

#include <algorithm>

namespace lifegame::domain
{

History::History(int interval)
    : m_interval(std::max(1, interval))
{
}

void History::reset(int generation, std::shared_ptr<const Grid> board)
{
    m_epoch = generation;
    m_snapshots.clear();
    if (board)
    {
        m_snapshots.push_back({generation, std::move(board)});
    }
}

void History::record(int generation, std::shared_ptr<const Grid> board)
{
    if (!board)
    {
        return;
    }
    if (m_snapshots.empty())
    {
        reset(generation, std::move(board));
        return;
    }
    if (generation <= m_snapshots.back().generation || !aligned(generation))
    {
        return;
    }

    m_snapshots.push_back({generation, std::move(board)});
    trim();
}

bool History::shouldRecord(int generation) const
{
    return !m_snapshots.empty() && generation > m_snapshots.back().generation &&
           aligned(generation);
}

void History::invalidateFrom(int generation)
{
    while (!m_snapshots.empty() && m_snapshots.back().generation >= generation)
    {
        m_snapshots.pop_back();
    }
}

int History::interval() const
{
    return m_interval;
}

void History::setInterval(int interval)
{
    m_interval = std::max(1, interval);
}

int History::epochGeneration() const
{
    return m_epoch;
}

int History::oldestGeneration() const
{
    return m_snapshots.empty() ? 0 : m_snapshots.front().generation;
}

const History::Snapshot* History::snapshotAtOrBefore(int generation) const
{
    for (auto it = m_snapshots.rbegin(); it != m_snapshots.rend(); ++it)
    {
        if (it->generation <= generation)
        {
            return &*it;
        }
    }
    return nullptr;
}

bool History::aligned(int generation) const
{
    return generation >= m_epoch && (generation - m_epoch) % m_interval == 0;
}

void History::trim()
{
    // Always keep the anchor (index 0); drop the oldest interior snapshot first.
    while (m_snapshots.size() > static_cast<std::size_t>(kMaxSnapshots) + 1u)
    {
        m_snapshots.erase(m_snapshots.begin() + 1);
    }
}

} // namespace lifegame::domain
