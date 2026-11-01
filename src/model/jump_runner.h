#pragma once

/**
 * @file jump_runner.h
 * @brief Background rewind/fast-forward replay.
 */

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "domain/grid.h"
#include "domain/packed_grid.h"
#include "domain/rule.h"
#include "model/command_result.h"

namespace lifegame::model
{

/// @brief One packed snapshot produced during replay.
struct JumpSnapshot
{
    int generation = 0; ///< Generation of the snapshot.
    std::shared_ptr<const domain::PackedGrid> packedBoard; ///< Packed board.
};

/// @brief Result of a background rewind/fast-forward replay.
struct JumpOutcome
{
    std::shared_ptr<const domain::Grid> board; ///< Final board copy.
    std::vector<JumpSnapshot> snapshots;        ///< New snapshots gathered while replaying.
    int generation = 0;                         ///< Target generation.
    bool cancelled = false;                     ///< True when the cancel flag was observed.
    CommandResult error = CommandResult::Success; ///< Set on allocation/computation failure.
};

/// @brief Immutable request for a replay worker.
struct JumpRequest
{
    std::shared_ptr<const domain::Grid> startBoard; ///< Board at startGeneration.
    int startGeneration = 0;  ///< First generation already represented.
    int targetGeneration = 0; ///< Requested final generation.
    int totalGenerations = 0; ///< target - start.
    bool forward = false;     ///< True when collecting new snapshots.
    domain::Rule rule;        ///< Rule used for replay.
    domain::Boundary boundary = domain::Boundary::Dead; ///< Boundary policy.
    int epoch = 0;            ///< Snapshot epoch.
    int interval = 100;       ///< Snapshot interval.
    std::shared_ptr<std::atomic_bool> cancel; ///< Cooperative cancellation flag.
    std::function<void(int)> reportProgress; ///< Optional progress callback (worker thread).
};

/**
 * @brief Run a rewind/fast-forward replay on the current worker thread.
 * @param request Immutable replay request.
 * @return Final board, new snapshots, or a typed error.
 * @note Caller must run this on a worker thread; progress callback is invoked
 *       there as well.
 */
JumpOutcome runJump(const JumpRequest& request) noexcept;

} // namespace lifegame::model
