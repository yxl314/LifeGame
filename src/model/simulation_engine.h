#pragma once

#include <QFutureWatcher>
#include <QObject>

#include <cstdint>
#include <memory>

#include "domain/boundary.h"
#include "domain/grid.h"
#include "domain/rule.h"
#include "model/command_result.h"

namespace lifegame::model
{

/**
 * Asynchronous one-generation computation.
 *
 * The engine owns the snapshot and the output buffer. `request()` returns a
 * monotonically increasing id; `stepReady(id)` is emitted on the UI thread when
 * the computation finishes. `discardPending()` makes the in-flight result
 * irrelevant (the worker still finishes, but no signal is emitted and a new
 * request is allowed once it returns).
 *
 * The engine owns the one-generation QtConcurrent pipeline; jump replay also
 * uses QtConcurrent, in LifeModel, but is kept separate from this class. This keeps
 * `LifeModel` focused on state and invariants.
 */
class SimulationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SimulationEngine(QObject* parent = nullptr);
    ~SimulationEngine() override;

    bool isBusy() const;

    /// Starts a request and writes its id. Returns Busy/OutOfBounds/OutOfMemory
    /// on failure; no exceptions are used.
    CommandResult request(const domain::Grid& board,
                          const domain::Rule& rule,
                          domain::Boundary boundary,
                          quint64* requestId);

    /// Invalidate the in-flight request; the worker is not interrupted.
    void discardPending();

    /// Move the finished generation out of the engine.
    domain::Grid takeResult();

signals:
    void stepReady(quint64 requestId);
    void stepFailed(lifegame::model::CommandResult reason);

private slots:
    void onFinished();

private:
    QFutureWatcher<int> m_watcher;
    domain::Grid m_output;
    std::shared_ptr<const domain::Grid> m_snapshot;
    quint64 m_activeId = 0;
    quint64 m_nextId = 1;
    bool m_busy = false;
    int m_expectedTasks = 0;
};

} // namespace lifegame::model
