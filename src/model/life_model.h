#pragma once

#include <QFutureWatcher>
#include <QObject>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain/grid.h"
#include "domain/history.h"
#include "domain/packed_grid.h"
#include "domain/pattern.h"
#include "domain/rle.h"
#include "domain/rule.h"
#include "shared/app_settings.h"
#include "model/command_result.h"
#include "shared/jump_info.h"
#include "shared/life_stats.h"
#include "model/jump_runner.h"
#include "model/simulation_engine.h"
#include "model/stall_reason.h"

Q_DECLARE_METATYPE(lifegame::shared::LifeStats)
Q_DECLARE_METATYPE(lifegame::shared::JumpInfo)

namespace lifegame::model
{

/**
 * Observable application model for Conway's Game of Life.
 *
 * Layered MVC contract:
 *  - Model knows nothing about views, services or controllers.
 *  - State changes are broadcast as signals; Controllers subscribe and push
 *    the resulting state into passive Views.
 *  - No file I/O, dialogs, QSettings or localized strings live here.
 *
 * All public methods run on the UI thread. The asynchronous step pipeline is
 * delegated to SimulationEngine; the asynchronous jump worker only reads a
 * snapshot and reports progress back through a queued invocation.
 */
class LifeModel : public QObject
{
    Q_OBJECT

public:
    explicit LifeModel(QObject* parent = nullptr);
    ~LifeModel() override;

    // ---- read-only queries (the view's entire model surface) ----
    const domain::Grid& board() const;
    int width() const;
    int height() const;
    shared::LifeStats stats() const;
    const domain::Rule& rule() const;
    domain::Boundary boundary() const;
    bool isRunning() const;
    bool isJumping() const;
    int oldestGeneration() const;
    int snapshotInterval() const;

    /// RLE text for the current board/rule (no file I/O).
    std::string toRle(std::string_view title = {}) const;

    // ---- commands (controllers only) ----
    /// Paint-path operations return "did anything change"; they have no
    /// user-facing failure reason beyond out-of-bounds/no-change.
    bool setCell(int row, int col, bool alive);
    bool toggleCell(int row, int col);

    /// Batch edit used by drag painting; emits at most one pair of signals.
    bool setCells(const std::vector<std::pair<int, int>>& cells, bool alive);
    CommandResult placePattern(const domain::Pattern& pattern, int centerRow, int centerCol);
    CommandResult clear();
    CommandResult randomize(int densityPercent);
    CommandResult resizeBoard(int width, int height);
    CommandResult setRule(const domain::Rule& rule);
    CommandResult setRuleByNotation(std::string_view notation);
    CommandResult setBoundary(domain::Boundary boundary);
    CommandResult setSnapshotInterval(int generations);

    /// Restore-only state forging (used by SessionController).
    CommandResult setGeneration(int generation);
    CommandResult loadDocument(const domain::DecodedDocument& document);

    /// Evolve one generation synchronously (single-step).
    CommandResult step();

    /// Request one asynchronous generation; returns Busy/NotRunning/Success.
    CommandResult requestStep();

    CommandResult setRunning(bool running);
    CommandResult toggleRunning();

    /// Jump forward (positive) or rewind (negative) by replaying snapshots.
    CommandResult startJump(int targetGeneration);
    void cancelJump();

signals:
    void boardChanged();
    void boardResized(int width, int height);
    void statusChanged(const lifegame::shared::LifeStats& stats);
    void runningChanged(bool running);
    void configurationChanged();
    void simulationStalled(lifegame::model::StallReason reason);
    void stepFailed(lifegame::model::CommandResult reason);

    void jumpStarted(const lifegame::shared::JumpInfo& info);
    void jumpProgress(int done);
    void jumpFinished(int generation);
    void jumpCancelled();
    void jumpFailed(lifegame::model::CommandResult reason);

private slots:
    void onStepReady(quint64 requestId);
    void onStepFailed(lifegame::model::CommandResult reason);
    void onJumpFinished();
    void reportJumpProgress(int done);

private:
    void pauseForEdit();
    void resetHistory();
    void recordSnapshot();
    void invalidateHistoryFrom(int generation);
    shared::LifeStats computeStats() const;
    void notifyBoardChanged();
    void notifyStatusChanged();
    void applyStep(domain::Grid&& next);
    CommandResult replaceBoard(std::unique_ptr<domain::Grid> board, int generation);
    bool canMutate() const;

    domain::Grid m_board;
    domain::Rule m_rule;
    domain::Boundary m_boundary = domain::Boundary::Dead;
    int m_generation = 0;
    int m_population = 0;
    bool m_running = false;

    domain::History m_history;
    SimulationEngine m_engine;
    quint64 m_pendingStepId = 0;

    QFutureWatcher<JumpOutcome> m_jumpWatcher;
    std::shared_ptr<std::atomic_bool> m_jumpCancel;
    bool m_jumping = false;
};

} // namespace lifegame::model
