#pragma once

/**
 * @file session_service.h
 * @brief UI-free session persistence service.
 */

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

/// @brief Result of SessionService::restore().
struct SessionRestoreResult
{
    shared::AppSettings settings;              ///< Normalized settings.
    service::SettingsError error = service::SettingsError::None; ///< Settings error.
    QString detail;                            ///< Optional error detail.
    bool restored = false;                     ///< Board restored successfully.
    bool hadStoredBoard = false;               ///< State contained board RLE.
    bool decodeFailed = false;                 ///< Stored RLE was malformed.
    bool loadFailed = false;                   ///< Model rejected decoded document.
    model::CommandResult loadResult = model::CommandResult::Success; ///< Model result.
    QByteArray windowGeometry;                 ///< Saved window geometry.
};

/// @brief UI-free session persistence.
class SessionService
{
public:
    /// @param model Model to restore/save.
    /// @param store Persistence store.
    SessionService(model::LifeModel* model, service::SettingsStore* store);

    /// @return Restore result and fallback board state.
    SessionRestoreResult restore();

    /**
     * @brief Persist current model state and settings.
     * @param settings Settings to store.
     * @param windowGeometry Window geometry to store.
     * @param detail Optional error detail.
     * @return Settings error code.
     */
    service::SettingsError save(const shared::AppSettings& settings,
                                const QByteArray& windowGeometry,
                                QString* detail = nullptr);

private:
    model::LifeModel* m_model = nullptr;
    service::SettingsStore* m_store = nullptr;
};

} // namespace lifegame::controller
