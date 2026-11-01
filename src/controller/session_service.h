#pragma once

#include <QByteArray>
#include <QString>

#include "model/command_result.h"
#include "service/settings_error.h"
#include "shared/app_settings.h"

namespace lifegame::domain
{
struct RleErrorInfo;
}

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::service
{
class SettingsStore;
}

namespace lifegame::controller
{

struct SessionRestoreResult
{
    shared::AppSettings settings;
    service::SettingsError error = service::SettingsError::None;
    QString detail;
    bool restored = false;
    bool hadStoredBoard = false;
    bool decodeFailed = false;
    bool loadFailed = false;
    model::CommandResult loadResult = model::CommandResult::Success;
    QByteArray windowGeometry;
};

/// Pure session persistence: no View, no dialogs, no messages.
class SessionService
{
public:
    SessionService(model::LifeModel* model, service::SettingsStore* store);

    SessionRestoreResult restore();
    service::SettingsError save(const shared::AppSettings& settings,
                                const QByteArray& windowGeometry,
                                QString* detail = nullptr);

private:
    model::LifeModel* m_model = nullptr;
    service::SettingsStore* m_store = nullptr;
};

} // namespace lifegame::controller
