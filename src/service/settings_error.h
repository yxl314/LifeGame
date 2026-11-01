#pragma once

/**
 * @file settings_error.h
 * @brief Typed settings persistence errors.
 */

namespace lifegame::service
{

enum class SettingsError
{
    None = 0,      ///< Success.
    AccessError,   ///< File cannot be read/written.
    FormatError,   ///< QSettings format is invalid.
    CorruptData    ///< One or more values are malformed/clamped.
};

constexpr const char* settingsErrorName(SettingsError error) noexcept
{
    switch (error)
    {
    case SettingsError::None:
        return "None";
    case SettingsError::AccessError:
        return "AccessError";
    case SettingsError::FormatError:
        return "FormatError";
    case SettingsError::CorruptData:
        return "CorruptData";
    }
    return "Unknown";
}

} // namespace lifegame::service
