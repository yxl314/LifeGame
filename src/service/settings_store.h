#pragma once

#include <QString>

#include <memory>

#include "service/session_state.h"
#include "service/settings_error.h"

class QSettings;

namespace lifegame::service
{

struct SettingsLoadResult
{
    SessionState state;
    SettingsError error = SettingsError::None;
    QString detail;
};

/// QSettings-backed persistence. No UI, no model state machine.
class SettingsStore
{
public:
    explicit SettingsStore(std::unique_ptr<QSettings> settings);
    ~SettingsStore();

    SettingsStore(const SettingsStore&) = delete;
    SettingsStore& operator=(const SettingsStore&) = delete;

    SettingsLoadResult load() const;
    SettingsError save(const SessionState& state, QString* detail = nullptr);

private:
    std::unique_ptr<QSettings> m_settings;
};

} // namespace lifegame::service
