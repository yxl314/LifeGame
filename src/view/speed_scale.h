#pragma once

/**
 * @file speed_scale.h
 * @brief Slider-position to generations-per-second mapping.
 */

namespace lifegame::view
{

inline constexpr int kSpeedSliderSteps = 100;

/// @param position Slider position in [0, 100].
/// @return Generations per second in [0, 120]; 0 means pause.
int speedForSliderPosition(int position);

/// @param speed Generations per second.
/// @return Slider position in [0, 100].
int sliderPositionForSpeed(int speed);

} // namespace lifegame::view
