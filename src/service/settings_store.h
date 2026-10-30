#pragma once

#include <memory>

class QSettings;

namespace lifegame::service
{

struct SessionState;

/// QSettings-backed persistence. No UI, no model state machine.
class SettingsStore
{
public:
    explicit SettingsStore(std::unique_ptr<QSettings> settings);
    ~SettingsStore();

    SettingsStore(const SettingsStore&) = delete;
    SettingsStore& operator=(const SettingsStore&) = delete;

    SessionState load() const;
    void save(const SessionState& state);

private:
    std::unique_ptr<QSettings> m_settings;
};

} // namespace lifegame::service
