#pragma once

/**
 * @file command_result.h
 * @brief Typed model-command results.
 */

#include <QMetaType>

namespace lifegame::model
{

/**
 * @brief Result of a model command.
 *
 * Application code never throws for expected failures; controllers switch on
 * this code and translate it into one actionable user message.
 */
enum class CommandResult
{
    Success = 0,       ///< Command completed exactly as requested.
    Busy,              ///< Model is repainting/jumping and rejected the command.
    OutOfBounds,       ///< Coordinate or index outside the board.
    NoChange,          ///< Command was valid but state was already identical.
    InvalidRule,       ///< Rule notation/mask is invalid.
    InvalidPattern,    ///< Pattern is empty or structurally inconsistent.
    InvalidArgument,   ///< Null/invalid API argument.
    InvalidTarget,     ///< Jump target is negative or equals current generation.
    NoHistory,         ///< Rewind has no snapshot at/before the target.
    EmptyBoard,        ///< Simulation cannot start on an empty board.
    NotRunning,        ///< Step requested while simulation is paused.
    Clipped,           ///< Pattern was placed but clipped by the board edge.
    Adjusted,          ///< Requested size was clamped to a supported range.
    TooLarge,          ///< Document/pattern exceeds board limits.
    OutOfMemory,       ///< Required allocation failed.
    ComputationFailed  ///< Background step/jump computation failed.
};

/// @param result Command result.
/// @return True for Success, Clipped and Adjusted (the command took effect).
constexpr bool succeeded(CommandResult result) noexcept
{
    return result == CommandResult::Success || result == CommandResult::Clipped ||
           result == CommandResult::Adjusted;
}

/// @param result Command result.
/// @return True when the command did not take effect.
constexpr bool failed(CommandResult result) noexcept
{
    return !succeeded(result);
}

/// @param result Command result.
/// @return Stable ASCII name for logs/tests (never shown to users).
constexpr const char* commandResultName(CommandResult result) noexcept
{
    switch (result)
    {
    case CommandResult::Success:
        return "Success";
    case CommandResult::Busy:
        return "Busy";
    case CommandResult::OutOfBounds:
        return "OutOfBounds";
    case CommandResult::NoChange:
        return "NoChange";
    case CommandResult::InvalidRule:
        return "InvalidRule";
    case CommandResult::InvalidPattern:
        return "InvalidPattern";
    case CommandResult::InvalidArgument:
        return "InvalidArgument";
    case CommandResult::InvalidTarget:
        return "InvalidTarget";
    case CommandResult::NoHistory:
        return "NoHistory";
    case CommandResult::EmptyBoard:
        return "EmptyBoard";
    case CommandResult::NotRunning:
        return "NotRunning";
    case CommandResult::Clipped:
        return "Clipped";
    case CommandResult::Adjusted:
        return "Adjusted";
    case CommandResult::TooLarge:
        return "TooLarge";
    case CommandResult::OutOfMemory:
        return "OutOfMemory";
    case CommandResult::ComputationFailed:
        return "ComputationFailed";
    }
    return "Unknown";
}

} // namespace lifegame::model

Q_DECLARE_METATYPE(lifegame::model::CommandResult)
