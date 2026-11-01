#pragma once

/**
 * @file boundary.h
 * @brief Finite-board boundary policy.
 */

namespace lifegame::domain
{

/// @brief How cells outside the finite board are treated.
enum class Boundary
{
    Dead = 0, ///< Outside the board is always dead.
    Wrap = 1  ///< The board wraps like a torus.
};

} // namespace lifegame::domain
