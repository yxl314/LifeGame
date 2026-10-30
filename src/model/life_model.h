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
#include "domain/pattern.h"
#include "domain/rle.h"
#include "domain/rule.h"
#include "model/app_settings.h"
#include "model/jump_info.h"
#include "model/life_stats.h"
#include "model/simulation_engine.h"
#include "model/stall_reason.h"

namespace lifegame::model
{

/**
 * Observable application model for Conway's Game of Life.
 *
 * Classic MVC contract:
 *  - Model knows nothing about views or controllers.
 *  - State changes are broadcast as signals; views read state through const
 *    queries only.
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
    LifeStats stats() const;
    const domain::Rule& rule() const;
    domain::Boundary boundary() const;
    bool isRunning() const;
    bool isJumping() const;
    int oldestGeneration() const;
    int snapshotInterval() const;

    /// RLE text for the current board/rule (no file I/O).
    std::string toRle(std::string_view title = {}) const;

    // ---- commands (controllers only) ----
    bool setCell(int row, int col, bool alive);
    bool toggleCell(int row, int col);

    /// Batch edit used by drag painting; emits at most one pair of signals.
    bool setCells(const std::vector<std::pair<int, int>>& cells, bool alive);
    bool placePattern(const domain::Pattern& pattern, int centerRow, int centerCol);
    bool clear();
    bool randomize(int densityPercent);
    bool resizeBoard(int width, int height);
    bool setRule(const domain::Rule& rule);
    bool setRuleByNotation(std::string_view notation);
    bool setBoundary(domain::Boundary boundary);
    bool setSnapshotInterval(int generations);
    bool setGeneration(int generation);
    bool loadDocument(const domain::DecodedDocument& document);

    /// Evolve one generation synchronously (single-step).
    void step();

    /// Request one asynchronous generation; ignored while busy or paused.
    void requestStep();

    bool setRunning(bool running);
    bool toggleRunning();

    /// Jump forward or rewind. Returns false when invalid or already jumping.
    bool startJump(int targetGeneration);
    void cancelJump();

signals:
    void boardChanged();
    void boardResized(int width, int height);
    void statusChanged(const lifegame::model::LifeStats& stats);
    void runningChanged(bool running);
    void configurationChanged();
    void simulationStalled(lifegame::model::StallReason reason);

    void jumpStarted(const lifegame::model::JumpInfo& info);
    void jumpProgress(int done);
    void jumpFinished(int generation);
    void jumpCancelled();

private slots:
    void onStepReady(quint64 requestId);
    void onJumpFinished();
    void reportJumpProgress(int done);

private:
    struct JumpOutcome
    {
        std::shared_ptr<const domain::Grid> board;
        std::vector<domain::History::Snapshot> snapshots;
        int generation = 0;
        bool cancelled = false;
    };

    void pauseForEdit();
    void resetHistory();
    void recordSnapshot();
    void invalidateHistoryFrom(int generation);
    LifeStats computeStats() const;
    void notifyBoardChanged();
    void notifyStatusChanged();
    void applyStep(domain::Grid&& next);
    bool replaceBoard(std::unique_ptr<domain::Grid> board, int generation);
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
