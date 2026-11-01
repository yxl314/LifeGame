#pragma once

#include <cstdint>

/**
 * @file cell.h
 * @brief Cell storage type and alive/dead values.
 */

namespace lifegame::domain
{

/**
 * @brief Compact cell storage: one byte per cell.
 *
 * One byte keeps the live board cache-friendly; packed history snapshots use
 * PackedGrid instead.
 */
using Cell = std::uint8_t;

inline constexpr Cell kDead = 0; ///< Dead cell value.
inline constexpr Cell kAlive = 1; ///< Alive cell value.

} // namespace lifegame::domain
