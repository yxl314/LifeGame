#pragma once

/**
 * @file stall_reason.h
 * @brief Reasons the simulation stops by itself.
 */

#include <QMetaType>

namespace lifegame::model
{

/// @brief Why the simulation stopped by itself.
enum class StallReason
{
    EmptyBoard = 0, ///< No alive cells.
    Extinct,        ///< All cells died during evolution.
    StillLife       ///< Board stopped changing.
};

/// @param reason Stall reason.
/// @return Stable ASCII name for logs/tests.
constexpr const char* stallReasonName(StallReason reason) noexcept
{
    switch (reason)
    {
    case StallReason::EmptyBoard:
        return "empty_board";
    case StallReason::Extinct:
        return "extinct";
    case StallReason::StillLife:
        return "still_life";
    }
    return "unknown";
}

} // namespace lifegame::model

Q_DECLARE_METATYPE(lifegame::model::StallReason)
