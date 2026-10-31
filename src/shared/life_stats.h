#pragma once

namespace lifegame::shared
{

/// Immutable snapshot of the statistics shown by the view.
struct LifeStats
{
    int generation = 0;
    int population = 0;
    double density = 0.0;
};

} // namespace lifegame::shared
