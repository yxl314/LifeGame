#pragma once

#include <QMetaType>

namespace lifegame::model
{

/// Result of a model command.
///
/// Application code must not throw for expected failures; controllers switch on
/// this code and translate it into one actionable user message.
enum class CommandResult
{
    Success = 0,
    Busy,
    OutOfBounds,
    NoChange,
    InvalidRule,
    InvalidPattern,
    InvalidArgument,
    InvalidTarget,
    NoHistory,
    EmptyBoard,
    NotRunning,
    Clipped,
    Adjusted,
    TooLarge,
    OutOfMemory,
    ComputationFailed
};

/// True for Success, Clipped and Adjusted: the command took effect.
constexpr bool succeeded(CommandResult result) noexcept
{
    return result == CommandResult::Success || result == CommandResult::Clipped ||
           result == CommandResult::Adjusted;
}

constexpr bool failed(CommandResult result) noexcept
{
    return !succeeded(result);
}

/// Stable ASCII name for logs and test diagnostics (never shown to users).
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
