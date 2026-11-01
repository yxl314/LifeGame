#pragma once

/**
 * @file jump_info.h
 * @brief Rewind/fast-forward progress value object.
 */

namespace lifegame::shared
{

/// @brief Parameters of a rewind/fast-forward operation.
struct JumpInfo
{
    int fromGeneration = 0;    ///< Generation where replay starts.
    int toGeneration = 0;      ///< Requested generation.
    int replayGenerations = 0; ///< Number of generations to replay.
};

} // namespace lifegame::shared
