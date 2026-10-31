#pragma once

namespace lifegame::service
{

enum class SettingsError
{
    None = 0,
    AccessError,
    FormatError,
    CorruptData
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
