#include "model/life_model.h"

#include <QMetaObject>
#include <QMetaType>
#include <QtConcurrent>

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>

namespace lifegame::model
{

LifeModel::LifeModel(QObject* parent)
    : QObject(parent),
      m_board(domain::kDefaultBoardWidth, domain::kDefaultBoardHeight),
      m_history(100)
{
    qRegisterMetaType<LifeStats>("lifegame::model::LifeStats");
    qRegisterMetaType<StallReason>("lifegame::model::StallReason");
    qRegisterMetaType<JumpInfo>("lifegame::model::JumpInfo");

    connect(&m_engine, &SimulationEngine::stepReady, this, &LifeModel::onStepReady);
    connect(&m_jumpWatcher, &QFutureWatcher<JumpOutcome>::finished, this, &LifeModel::onJumpFinished);

    resetHistory();
}

LifeModel::~LifeModel()
{
    if (m_jumpCancel)
    {
        *m_jumpCancel = true;
    }
    disconnect(&m_jumpWatcher, nullptr, this, nullptr);
    if (m_jumpWatcher.isRunning())
    {
        m_jumpWatcher.waitForFinished();
    }
}

const domain::Grid& LifeModel::board() const
{
    return m_board;
}

int LifeModel::width() const
{
    return m_board.width();
}

int LifeModel::height() const
{
    return m_board.height();
}

LifeStats LifeModel::stats() const
{
    return computeStats();
}

const domain::Rule& LifeModel::rule() const
{
    return m_rule;
}

domain::Boundary LifeModel::boundary() const
{
    return m_boundary;
}

bool LifeModel::isRunning() const
{
    return m_running;
}

bool LifeModel::isJumping() const
{
    return m_jumping;
}

int LifeModel::oldestGeneration() const
{
    return m_history.empty() ? m_generation : m_history.oldestGeneration();
}

int LifeModel::snapshotInterval() const
{
    return m_history.interval();
}

std::string LifeModel::toRle(std::string_view title) const
{
    return domain::encodeRle(m_board, m_rule, title);
}

bool LifeModel::canMutate() const
{
    return !m_jumping;
}

void LifeModel::pauseForEdit()
{
    m_engine.discardPending();
    m_pendingStepId = 0;

    if (m_running)
    {
        m_running = false;
        emit runningChanged(false);
    }
}

bool LifeModel::setRunning(bool running)
{
    if (m_jumping)
    {
        return false;
    }
    if (!running)
    {
        m_engine.discardPending();
        m_pendingStepId = 0;
    }
    if (m_running == running)
    {
        return true;
    }
    if (running && m_population == 0)
    {
        emit simulationStalled(StallReason::EmptyBoard);
        return false;
    }

    m_running = running;
    emit runningChanged(running);
    return true;
}

bool LifeModel::toggleRunning()
{
    return setRunning(!m_running);
}

bool LifeModel::setCell(int row, int col, bool alive)
{
    if (!canMutate() || !m_board.inBounds(row, col) || m_board.cell(row, col) == alive)
    {
        return false;
    }

    pauseForEdit();
    m_board.setCell(row, col, alive);
    m_population += alive ? 1 : -1;
    invalidateHistoryFrom(m_generation);
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::toggleCell(int row, int col)
{
    if (!canMutate() || !m_board.inBounds(row, col))
    {
        return false;
    }

    pauseForEdit();
    const bool alive = m_board.toggleCell(row, col);
    m_population += alive ? 1 : -1;
    invalidateHistoryFrom(m_generation);
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::setCells(const std::vector<std::pair<int, int>>& cells, bool alive)
{
    if (!canMutate() || cells.empty())
    {
        return false;
    }

    bool wouldChange = false;
    for (const auto& cell : cells)
    {
        if (m_board.inBounds(cell.first, cell.second) && m_board.cell(cell.first, cell.second) != alive)
        {
            wouldChange = true;
            break;
        }
    }
    if (!wouldChange)
    {
        return false;
    }

    pauseForEdit();
    for (const auto& cell : cells)
    {
        m_board.setCell(cell.first, cell.second, alive);
    }
    m_population = m_board.population();
    invalidateHistoryFrom(m_generation);
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::placePattern(const domain::Pattern& pattern, int centerRow, int centerCol)
{
    if (!canMutate() || pattern.width <= 0 || pattern.height <= 0 || pattern.population() == 0)
    {
        return false;
    }

    const int topRow = centerRow - pattern.height / 2;
    const int topCol = centerCol - pattern.width / 2;

    bool wouldChange = false;
    for (int row = 0; row < pattern.height && !wouldChange; ++row)
    {
        for (int col = 0; col < pattern.width; ++col)
        {
            if (pattern.alive(row, col) && !m_board.cell(topRow + row, topCol + col))
            {
                wouldChange = true;
                break;
            }
        }
    }
    if (!wouldChange)
    {
        return false;
    }

    pauseForEdit();
    for (int row = 0; row < pattern.height; ++row)
    {
        for (int col = 0; col < pattern.width; ++col)
        {
            if (pattern.alive(row, col))
            {
                m_board.setCell(topRow + row, topCol + col, true);
            }
        }
    }
    m_population = m_board.population();
    invalidateHistoryFrom(m_generation);
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::clear()
{
    if (!canMutate())
    {
        return false;
    }

    pauseForEdit();
    m_board.clear();
    m_population = 0;
    m_generation = 0;
    resetHistory();
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::randomize(int densityPercent)
{
    if (!canMutate())
    {
        return false;
    }

    const int density = std::clamp(densityPercent, domain::kMinRandomDensity, domain::kMaxRandomDensity);
    std::random_device device;
    pauseForEdit();
    m_board.randomize(density, device());
    m_population = m_board.population();
    m_generation = 0;
    resetHistory();
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::resizeBoard(int width, int height)
{
    if (!canMutate())
    {
        return false;
    }

    const int clampedWidth = std::clamp(width, domain::kMinBoardExtent, domain::kMaxBoardExtent);
    const int clampedHeight = std::clamp(height, domain::kMinBoardExtent, domain::kMaxBoardExtent);
    auto board = std::make_unique<domain::Grid>(clampedWidth, clampedHeight);
    return replaceBoard(std::move(board), 0);
}

bool LifeModel::setRule(const domain::Rule& rule)
{
    if (!canMutate() || m_rule == rule)
    {
        return false;
    }

    m_engine.discardPending();
    m_pendingStepId = 0;
    m_rule = rule;
    resetHistory();
    emit configurationChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::setRuleByNotation(std::string_view notation)
{
    const auto rule = domain::Rule::fromNotation(notation);
    if (!rule.has_value())
    {
        return false;
    }
    return setRule(*rule);
}

bool LifeModel::setBoundary(domain::Boundary boundary)
{
    if (!canMutate() || m_boundary == boundary)
    {
        return false;
    }

    m_engine.discardPending();
    m_pendingStepId = 0;
    m_boundary = boundary;
    resetHistory();
    emit configurationChanged();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::setSnapshotInterval(int generations)
{
    if (!canMutate())
    {
        return false;
    }

    const int interval =
        std::clamp(generations, limits::kSnapshotIntervalMin, limits::kSnapshotIntervalMax);
    if (interval == m_history.interval())
    {
        return false;
    }

    m_engine.discardPending();
    m_pendingStepId = 0;
    m_history.setInterval(interval);
    resetHistory();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::setGeneration(int generation)
{
    if (!canMutate())
    {
        return false;
    }

    const int value = std::max(0, generation);
    if (value == m_generation)
    {
        return false;
    }

    m_engine.discardPending();
    m_pendingStepId = 0;
    m_generation = value;
    resetHistory();
    emit statusChanged(stats());
    return true;
}

bool LifeModel::loadDocument(const domain::DecodedDocument& document)
{
    if (!canMutate())
    {
        return false;
    }

    const domain::Pattern& pattern = document.pattern;
    if (pattern.width <= 0 || pattern.height <= 0 || pattern.width > domain::kMaxBoardExtent ||
        pattern.height > domain::kMaxBoardExtent)
    {
        return false;
    }

    std::optional<domain::Rule> rule;
    if (!document.rule.empty())
    {
        rule = domain::Rule::fromNotation(document.rule);
        if (!rule.has_value())
        {
            return false;
        }
    }

    const int width = std::max(domain::kMinBoardExtent, pattern.width);
    const int height = std::max(domain::kMinBoardExtent, pattern.height);
    auto board = std::make_unique<domain::Grid>(width, height);
    for (int row = 0; row < pattern.height; ++row)
    {
        for (int col = 0; col < pattern.width; ++col)
        {
            if (pattern.alive(row, col))
            {
                board->setCell(row, col, true);
            }
        }
    }

    if (!replaceBoard(std::move(board), 0))
    {
        return false;
    }

    if (rule.has_value() && *rule != m_rule)
    {
        m_rule = *rule;
        emit configurationChanged();
    }
    return true;
}

void LifeModel::step()
{
    if (!canMutate())
    {
        return;
    }

    m_engine.discardPending();
    m_pendingStepId = 0;

    domain::Grid next;
    domain::Grid::computeNext(m_board, &next, m_rule, m_boundary);
    applyStep(std::move(next));
}

void LifeModel::requestStep()
{
    if (!m_running || m_jumping || m_engine.isBusy() || m_pendingStepId != 0)
    {
        return;
    }

    const quint64 requestId = m_engine.request(m_board, m_rule, m_boundary);
    if (requestId != 0)
    {
        m_pendingStepId = requestId;
    }
}

void LifeModel::onStepReady(quint64 requestId)
{
    if (requestId == 0 || requestId != m_pendingStepId)
    {
        return;
    }
    m_pendingStepId = 0;

    if (!m_running)
    {
        return;
    }

    applyStep(m_engine.takeResult());
}

void LifeModel::applyStep(domain::Grid&& next)
{
    const bool unchanged = next.equals(m_board);
    m_board = std::move(next);
    ++m_generation;
    m_population = m_board.population();
    recordSnapshot();

    emit boardChanged();
    emit statusChanged(stats());

    if (unchanged && m_running)
    {
        setRunning(false);
        emit simulationStalled(m_population == 0 ? StallReason::Extinct : StallReason::StillLife);
    }
}

bool LifeModel::startJump(int targetGeneration)
{
    if (!canMutate() || targetGeneration < 0 || targetGeneration == m_generation)
    {
        return false;
    }

    const bool forward = targetGeneration > m_generation;
    int startGeneration = 0;
    std::shared_ptr<const domain::Grid> startBoard;

    if (forward)
    {
        startGeneration = m_generation;
        startBoard = std::make_shared<const domain::Grid>(m_board);
    }
    else
    {
        const domain::History::Snapshot* snapshot = m_history.snapshotAtOrBefore(targetGeneration);
        if (snapshot == nullptr)
        {
            return false; // nothing to pause for
        }
        startGeneration = snapshot->generation;
        startBoard = snapshot->board;
    }

    const int total = targetGeneration - startGeneration;
    if (total < 0)
    {
        return false;
    }

    pauseForEdit();
    m_engine.discardPending();
    m_pendingStepId = 0;

    const int epoch = m_history.epochGeneration();
    const int interval = m_history.interval();
    const domain::Rule rule = m_rule;
    const domain::Boundary boundary = m_boundary;
    auto cancel = std::make_shared<std::atomic_bool>(false);
    m_jumpCancel = cancel;
    m_jumping = true;

    const int target = targetGeneration;
    const int start = startGeneration;
    const QFuture<JumpOutcome> future = QtConcurrent::run(
        [this, startBoard, start, target, total, forward, rule, boundary, epoch, interval, cancel]() {
            JumpOutcome outcome;
            outcome.generation = target;

            auto current = std::make_shared<domain::Grid>(*startBoard);
            auto next = std::make_shared<domain::Grid>(startBoard->width(), startBoard->height());

            for (int generation = start + 1; generation <= target; ++generation)
            {
                domain::Grid::computeNext(*current, next.get(), rule, boundary);
                std::swap(current, next);

                if (cancel->load())
                {
                    outcome.cancelled = true;
                    return outcome;
                }

                const int done = generation - start;

                if (forward && (generation - epoch) % interval == 0)
                {
                    outcome.snapshots.push_back(
                        {generation, std::make_shared<const domain::Grid>(*current)});
                    if (outcome.snapshots.size() > static_cast<std::size_t>(domain::History::kMaxSnapshots))
                    {
                        outcome.snapshots.erase(outcome.snapshots.begin());
                    }
                }

                // Once the board is empty it stays empty; the remaining
                // generations cannot change anything.
                if (done % 64 == 0 && current->population() == 0)
                {
                    break;
                }

                if (done % 64 == 0)
                {
                    QMetaObject::invokeMethod(this,
                                              "reportJumpProgress",
                                              Qt::QueuedConnection,
                                              Q_ARG(int, done));
                }
            }

            outcome.board = current;
            return outcome;
        });

    m_jumpWatcher.setFuture(future);
    emit jumpStarted(JumpInfo{m_generation, target, total});
    return true;
}

void LifeModel::cancelJump()
{
    if (m_jumpCancel)
    {
        *m_jumpCancel = true;
    }
}

void LifeModel::reportJumpProgress(int done)
{
    emit jumpProgress(done);
}

void LifeModel::onJumpFinished()
{
    m_jumping = false;
    m_jumpCancel.reset();

    const JumpOutcome outcome = m_jumpWatcher.result();
    if (outcome.cancelled || !outcome.board)
    {
        emit jumpCancelled();
        return;
    }

    const int previousGeneration = m_generation;
    m_board = *outcome.board;
    m_generation = outcome.generation;
    m_population = m_board.population();

    if (m_generation < previousGeneration)
    {
        m_history.invalidateFrom(m_generation + 1);
    }
    else
    {
        for (const domain::History::Snapshot& snapshot : outcome.snapshots)
        {
            m_history.record(snapshot.generation, snapshot.board);
        }
    }

    if (m_history.empty())
    {
        resetHistory();
    }

    emit boardChanged();
    emit statusChanged(stats());
    emit jumpFinished(m_generation);
}

void LifeModel::resetHistory()
{
    m_history.reset(m_generation, std::make_shared<const domain::Grid>(m_board));
}

void LifeModel::recordSnapshot()
{
    if (m_history.shouldRecord(m_generation))
    {
        m_history.record(m_generation, std::make_shared<const domain::Grid>(m_board));
    }
}

void LifeModel::invalidateHistoryFrom(int generation)
{
    m_history.invalidateFrom(generation);
    if (m_history.empty())
    {
        resetHistory();
    }
}

LifeStats LifeModel::computeStats() const
{
    const int area = m_board.width() * m_board.height();
    const double raw = area > 0 ? 100.0 * static_cast<double>(m_population) / static_cast<double>(area) : 0.0;
    return LifeStats{m_generation, m_population, std::round(raw * 10.0) / 10.0};
}

bool LifeModel::replaceBoard(std::unique_ptr<domain::Grid> board, int generation)
{
    if (!board || !canMutate())
    {
        return false;
    }

    const bool sizeChanged =
        board->width() != m_board.width() || board->height() != m_board.height();

    pauseForEdit();
    m_engine.discardPending();
    m_pendingStepId = 0;

    m_board = std::move(*board);
    m_generation = generation;
    m_population = m_board.population();
    resetHistory();

    if (sizeChanged)
    {
        emit boardResized(m_board.width(), m_board.height());
    }
    emit boardChanged();
    emit statusChanged(stats());
    return true;
}

} // namespace lifegame::model
