#pragma once

#include <QMetaType>

namespace lifegame::model
{

/// Immutable snapshot of the statistics shown by the view.
struct LifeStats
{
    int generation = 0;
    int population = 0;
    double density = 0.0;
};

} // namespace lifegame::model

Q_DECLARE_METATYPE(lifegame::model::LifeStats)
