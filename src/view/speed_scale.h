#pragma once

namespace lifegame::view
{

inline constexpr int kSpeedSliderSteps = 100;

/// Map slider position [0, 100] to generations/second [1, 200] (log scale).
int speedForSliderPosition(int position);

/// Inverse mapping, used only when loading settings (never during a drag).
int sliderPositionForSpeed(int speed);

} // namespace lifegame::view
