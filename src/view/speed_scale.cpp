#include "view/speed_scale.h"

#include <algorithm>
#include <cmath>

#include "shared/app_settings.h"

namespace lifegame::view
{

namespace
{

constexpr double kMinSpeed = static_cast<double>(shared::limits::kSpeedMin);
constexpr double kMaxSpeed = static_cast<double>(shared::limits::kSpeedMax);

} // namespace

int speedForSliderPosition(int position)
{
    const double ratio =
        static_cast<double>(std::clamp(position, 0, kSpeedSliderSteps)) / static_cast<double>(kSpeedSliderSteps);
    const double value = kMinSpeed * std::pow(kMaxSpeed / kMinSpeed, ratio);
    return std::clamp(static_cast<int>(std::lround(value)), shared::limits::kSpeedMin, shared::limits::kSpeedMax);
}

int sliderPositionForSpeed(int speed)
{
    const int clamped = std::clamp(speed, shared::limits::kSpeedMin, shared::limits::kSpeedMax);
    const double ratio = std::log(static_cast<double>(clamped) / kMinSpeed) / std::log(kMaxSpeed / kMinSpeed);
    return std::clamp(static_cast<int>(std::lround(ratio * kSpeedSliderSteps)), 0, kSpeedSliderSteps);
}

} // namespace lifegame::view
