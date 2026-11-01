#pragma once

#include <algorithm>

namespace lifegame::shared
{

/// Ranges shared by the settings dialog, the session store and validation.
namespace limits
{
inline constexpr int kJumpStepMin = 100;
inline constexpr int kJumpStepMax = 1000;
inline constexpr int kSizeStepMin = 5;
inline constexpr int kSizeStepMax = 50;
inline constexpr int kSnapshotIntervalMin = 10;
inline constexpr int kSnapshotIntervalMax = 1000;
inline constexpr int kSpeedMin = 0;
inline constexpr int kSpeedMax = 120;
inline constexpr int kDefaultSpeed = 10;
inline constexpr int kDensityMin = 5;
inline constexpr int kDensityMax = 90;

// Above this cell count the View uses the image path and the controller
// may coalesce repaints when the screen cannot keep up with the generation rate.
inline constexpr int kLargeBoardCellThreshold = 250000;
} // namespace limits

/// User-adjustable application settings (persisted with the session).
struct AppSettings
{
    int jumpStep = 100;
    int sizeStep = 5;
    int snapshotInterval = 100;
    int density = 30;
    int speed = limits::kDefaultSpeed;
    bool gridVisible = true;
};

constexpr shared::AppSettings normalized(const shared::AppSettings& settings) noexcept
{
    shared::AppSettings result = settings;
    result.jumpStep = std::clamp(result.jumpStep, limits::kJumpStepMin, limits::kJumpStepMax);
    result.sizeStep = std::clamp(result.sizeStep, limits::kSizeStepMin, limits::kSizeStepMax);
    result.snapshotInterval =
        std::clamp(result.snapshotInterval, limits::kSnapshotIntervalMin, limits::kSnapshotIntervalMax);
    result.density = std::clamp(result.density, limits::kDensityMin, limits::kDensityMax);
    result.speed = std::clamp(result.speed, limits::kSpeedMin, limits::kSpeedMax);
    return result;
}

} // namespace lifegame::shared
