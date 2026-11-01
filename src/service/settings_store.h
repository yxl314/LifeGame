#pragma once

/**
 * @file settings_store.h
 * @brief QSettings-backed persistence.
 */

#include <QString>

#include <memory>

#include "service/session_state.h"
#include "service/settings_error.h"

class QSettings;

namespace lifegame::service
{

/// @brief Result of loading persisted session data.
struct SettingsLoadResult
{
    SessionState state;                       ///< Loaded/normalized state.
    SettingsError error = SettingsError::None; ///< Load error, if any.
    QString detail;                           ///< Optional file/detail text.
};

/// QSettings-backed persistence. No UI, no model state machine.
class SettingsStore
{
public:
    /// @param settings Owning QSettings instance (may be null only in tests).
    explicit SettingsStore(std::unique_ptr<QSettings> settings);
    ~SettingsStore();

    SettingsStore(const SettingsStore&) = delete;
    SettingsStore& operator=(const SettingsStore&) = delete;

    /// @return Loaded state, defaults, and a typed error if applicable.
    SettingsLoadResult load() const;

    /**
     * @brief Persist session state.
     * @param state State to write.
     * @param detail Optional file/detail output.
     * @return SettingsError code.
     */
    SettingsError save(const SessionState& state, QString* detail = nullptr);

private:
    std::unique_ptr<QSettings> m_settings;
};

} // namespace lifegame::service
