#pragma once

namespace lifegame::domain
{

/// How cells outside the finite board are treated.
enum class Boundary
{
    /// Outside the board is always dead.
    Dead = 0,

    /// The board wraps like a torus.
    Wrap = 1
};

} // namespace lifegame::domain
