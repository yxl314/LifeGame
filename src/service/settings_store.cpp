#include "service/settings_store.h"

#include <QSettings>

#include <utility>

#include "service/session_state.h"

namespace lifegame::service
{

SettingsStore::SettingsStore(std::unique_ptr<QSettings> settings)
    : m_settings(std::move(settings))
{
}

SettingsStore::~SettingsStore() = default;

SessionState SettingsStore::load() const
{
    SessionState state;
    if (!m_settings)
    {
        return state;
    }

    state.settings.jumpStep = m_settings->value(QStringLiteral("settings/jump_step"), 100).toInt();
    state.settings.sizeStep = m_settings->value(QStringLiteral("settings/size_step"), 5).toInt();
    state.settings.snapshotInterval =
        m_settings->value(QStringLiteral("settings/snapshot_interval"), 100).toInt();
    state.settings.density = m_settings->value(QStringLiteral("simulation/density"), 30).toInt();
    state.settings.speed = m_settings->value(QStringLiteral("simulation/speed"), 10).toInt();
    state.settings.gridVisible = m_settings->value(QStringLiteral("view/grid"), true).toBool();
    state.settings = model::normalized(state.settings);

    state.boardRle = m_settings->value(QStringLiteral("board/rle")).toString();
    state.ruleNotation = m_settings->value(QStringLiteral("simulation/rule"), QStringLiteral("B3/S23")).toString();
    state.boundary = m_settings->value(QStringLiteral("simulation/boundary"), 0).toInt();
    state.generation = m_settings->value(QStringLiteral("board/generation"), 0).toInt();
    state.windowGeometry = m_settings->value(QStringLiteral("window/geometry")).toByteArray();
    return state;
}

void SettingsStore::save(const SessionState& state)
{
    if (!m_settings)
    {
        return;
    }

    m_settings->setValue(QStringLiteral("settings/jump_step"), state.settings.jumpStep);
    m_settings->setValue(QStringLiteral("settings/size_step"), state.settings.sizeStep);
    m_settings->setValue(QStringLiteral("settings/snapshot_interval"), state.settings.snapshotInterval);
    m_settings->setValue(QStringLiteral("simulation/density"), state.settings.density);
    m_settings->setValue(QStringLiteral("simulation/speed"), state.settings.speed);
    m_settings->setValue(QStringLiteral("simulation/rule"), state.ruleNotation);
    m_settings->setValue(QStringLiteral("simulation/boundary"), state.boundary);
    m_settings->setValue(QStringLiteral("board/rle"), state.boardRle);
    m_settings->setValue(QStringLiteral("board/generation"), state.generation);
    m_settings->setValue(QStringLiteral("view/grid"), state.settings.gridVisible);
    m_settings->setValue(QStringLiteral("window/geometry"), state.windowGeometry);
    m_settings->sync();
}

} // namespace lifegame::service
