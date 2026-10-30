#pragma once

#include <cstdint>

namespace lifegame::domain
{

/// Cell storage type: one byte keeps the fixed board compact and cache friendly.
using Cell = std::uint8_t;

inline constexpr Cell kDead = 0;
inline constexpr Cell kAlive = 1;

} // namespace lifegame::domain
