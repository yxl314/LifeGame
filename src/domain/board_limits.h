#pragma once

/**
 * @file board_limits.h
 * @brief Shared board, density and speed limits.
 */

namespace lifegame::domain
{

/**
 * @brief Board limits shared by domain, model, view and persistence.
 *
 * Keeping these values in one header prevents UI ranges, RLE limits and
 * storage capacity from drifting apart.
 */
inline constexpr int kMinBoardExtent = 20; ///< Minimum width and height in cells.
inline constexpr int kMaxBoardWidth = 1500; ///< Maximum width (x axis) in cells.
inline constexpr int kMaxBoardHeight = 1000; ///< Maximum height (y axis) in cells.

inline constexpr int kDefaultBoardWidth = 250; ///< Default board width.
inline constexpr int kDefaultBoardHeight = 200; ///< Default board height.

inline constexpr int kMinRandomDensity = 5; ///< Minimum random-fill percentage.
inline constexpr int kMaxRandomDensity = 90; ///< Maximum random-fill percentage.
inline constexpr int kDefaultRandomDensity = 30; ///< Default random-fill percentage.

inline constexpr int kMaxCells = kMaxBoardWidth * kMaxBoardHeight; ///< Maximum active cells.

} // namespace lifegame::domain
