#pragma once

#include <QMetaType>

namespace lifegame::model
{

/// Why the simulation stopped by itself. Localized text belongs to the view.
enum class StallReason
{
    EmptyBoard = 0,
    Extinct,
    StillLife
};

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
