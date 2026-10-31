#pragma once

namespace lifegame::shared
{

/// Parameters of a rewind/fast-forward operation.
struct JumpInfo
{
    int fromGeneration = 0;
    int toGeneration = 0;
    int replayGenerations = 0;
};

} // namespace lifegame::shared
