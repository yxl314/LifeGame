#pragma once

/**
 * @file life_stats.h
 * @brief Read-only population statistics.
 */

namespace lifegame::shared
{

/// @brief Immutable snapshot of the statistics shown by the view.
struct LifeStats
{
    int generation = 0;   ///< Current generation number.
    int population = 0;   ///< Number of alive cells.
    double density = 0.0; ///< Percentage of alive cells in [0, 100].
};

} // namespace lifegame::shared
