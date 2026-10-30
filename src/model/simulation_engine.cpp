#include "model/simulation_engine.h"

#include <QPair>
#include <QThreadPool>
#include <QVector>
#include <QtConcurrent>

#include <algorithm>
#include <utility>

namespace lifegame::model
{

namespace
{

constexpr int kParallelThreshold = 20000;
constexpr int kMinRowsPerChunk = 8;

/**
 * Functor passed to QtConcurrent::mapped.
 *
 * Qt 5 reads `result_type` from the functor, hence the explicit typedef
 * instead of a lambda.
 */
struct ComputeRowsTask
{
    using result_type = int;

    std::shared_ptr<const domain::Grid> snapshot;
    domain::Grid* output = nullptr;
    domain::Rule rule;
    domain::Boundary boundary = domain::Boundary::Dead;

    int operator()(const QPair<int, int>& range) const
    {
        domain::Grid::computeRows(*snapshot, output, rule, boundary, range.first, range.second);
        return 1;
    }
};

} // namespace

SimulationEngine::SimulationEngine(QObject* parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<int>::finished, this, &SimulationEngine::onFinished);
}

SimulationEngine::~SimulationEngine()
{
    disconnect(&m_watcher, nullptr, this, nullptr);
    if (m_watcher.isRunning())
    {
        m_watcher.waitForFinished();
    }
}

bool SimulationEngine::isBusy() const
{
    return m_busy;
}

quint64 SimulationEngine::request(const domain::Grid& board,
                                  const domain::Rule& rule,
                                  domain::Boundary boundary)
{
    if (m_busy)
    {
        return 0;
    }

    const int width = board.width();
    const int height = board.height();
    if (width <= 0 || height <= 0)
    {
        return 0;
    }

    m_snapshot = std::make_shared<const domain::Grid>(board);
    m_output.resize(width, height);
    m_activeId = m_nextId++;
    m_busy = true;

    QVector<QPair<int, int>> ranges;
    int chunks = QThreadPool::globalInstance()->maxThreadCount();
    chunks = std::max(1, chunks);
    if (width * height < kParallelThreshold)
    {
        chunks = 1;
    }
    chunks = std::min(chunks, std::max(1, height / kMinRowsPerChunk));

    if (chunks <= 1)
    {
        ranges.append(qMakePair(0, height));
    }
    else
    {
        const int chunkRows = (height + chunks - 1) / chunks;
        for (int begin = 0; begin < height; begin += chunkRows)
        {
            ranges.append(qMakePair(begin, std::min(height, begin + chunkRows)));
        }
    }

    const QFuture<int> future =
        QtConcurrent::mapped(ranges, ComputeRowsTask{m_snapshot, &m_output, rule, boundary});
    m_watcher.setFuture(future);
    return m_activeId;
}

void SimulationEngine::discardPending()
{
    if (!m_busy)
    {
        return;
    }
    m_activeId = 0;
}

domain::Grid SimulationEngine::takeResult()
{
    return std::move(m_output);
}

void SimulationEngine::onFinished()
{
    m_busy = false;
    const quint64 finishedId = m_activeId;
    m_activeId = 0;
    if (finishedId != 0)
    {
        emit stepReady(finishedId);
    }
}

} // namespace lifegame::model
