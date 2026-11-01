#pragma once

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

struct JumpSnapshot
{
    int generation = 0;
    std::shared_ptr<const domain::PackedGrid> packedBoard;
};

struct JumpOutcome
{
    std::shared_ptr<const domain::Grid> board;
    std::vector<JumpSnapshot> snapshots;
    int generation = 0;
    bool cancelled = false;
    CommandResult error = CommandResult::Success;
};

struct JumpRequest
{
    std::shared_ptr<const domain::Grid> startBoard;
    int startGeneration = 0;
    int targetGeneration = 0;
    int totalGenerations = 0;
    bool forward = false;
    domain::Rule rule;
    domain::Boundary boundary = domain::Boundary::Dead;
    int epoch = 0;
    int interval = 100;
    std::shared_ptr<std::atomic_bool> cancel;
    std::function<void(int)> reportProgress;
};

/// Pure background rewind/fast-forward replay. Runs on a worker thread.
JumpOutcome runJump(const JumpRequest& request) noexcept;

} // namespace lifegame::model
