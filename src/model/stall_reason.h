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

} // namespace lifegame::model

Q_DECLARE_METATYPE(lifegame::model::StallReason)
