#pragma once

namespace lifegame::domain
{

// Board limits are shared by the domain, model, view and persistence code so
// that there is exactly one source of truth.

inline constexpr int kMinBoardExtent = 20;
inline constexpr int kMaxBoardExtent = 400;

inline constexpr int kDefaultBoardWidth = 250;
inline constexpr int kDefaultBoardHeight = 200;

inline constexpr int kMinRandomDensity = 5;
inline constexpr int kMaxRandomDensity = 90;
inline constexpr int kDefaultRandomDensity = 30;

inline constexpr int kMaxCells = kMaxBoardExtent * kMaxBoardExtent;

} // namespace lifegame::domain
