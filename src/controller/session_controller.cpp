#include "controller/session_controller.h"

#include "controller/controller_log.h"
#include "controller/error_text.h"
#include "controller/session_service.h"

#include <QMainWindow>

#include "controller/board_controller.h"
#include "controller/simulation_controller.h"
#include "domain/rle.h"
#include "model/life_model.h"
#include "service/session_state.h"
#include "service/settings_store.h"
#include "view/control_panel.h"
#include "view/interfaces.h"

namespace lifegame::controller
{

SessionController::SessionController(model::LifeModel* model,
                                     view::ControlPanel* panel,
                                     SimulationController* simulation,
                                     BoardController* board,
                                     service::SettingsStore* store,
                                     QMainWindow* window,
                                     view::MessageSink* messages,
                                     QObject* parent)
    : QObject(parent),
      m_model(model),
      m_panel(panel),
      m_simulation(simulation),
      m_board(board),
      m_service(std::make_unique<SessionService>(model, store)),
      m_window(window),
      m_messages(messages)
{
}

SessionController::~SessionController() = default;

const shared::AppSettings& SessionController::settings() const
{
    return m_settings;
}

bool SessionController::restore()
{
    const SessionRestoreResult result = m_service->restore();
    m_settings = result.settings;

    qCInfo(lcController) << "session restore start has_stored_board=" << result.hadStoredBoard
                         << "restored=" << result.restored;
    if (result.error != service::SettingsError::None)
    {
        qCWarning(lcController) << "settings restore failed error="
                                << service::settingsErrorName(result.error);
        if (m_messages != nullptr)
        {
            m_messages->showMessage(settingsErrorText(result.error, result.detail), 8000);
        }
    }

    if (result.hadStoredBoard && !result.restored)
    {
        if (result.decodeFailed)
        {
            qCWarning(lcController) << "stored RLE decode failed";
            if (m_messages != nullptr)
            {
                m_messages->showMessage(tr("上次会话的棋盘无法解析，已改为示例棋盘。"), 8000);
            }
        }
        else if (result.loadFailed && m_messages != nullptr)
        {
            m_messages->showMessage(
                tr("上次会话的棋盘无法恢复：%1 已改为示例棋盘。")
                    .arg(commandResultText(result.loadResult)),
                8000);
        }
    }

    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
    m_panel->setRandomDensity(m_settings.density);
    m_panel->setSpeed(m_settings.speed);
    m_simulation->setSpeedExternal(m_settings.speed);
    m_board->setGridVisible(m_settings.gridVisible);

    if (m_window != nullptr && !result.windowGeometry.isEmpty() &&
        !m_window->restoreGeometry(result.windowGeometry))
    {
        qCDebug(lcController) << "window geometry restore failed";
    }

    qCInfo(lcController) << "session restore done restored=" << result.restored << "board="
                         << m_model->width() << 'x' << m_model->height() << "generation"
                         << m_model->stats().generation;
    return result.restored;
}

service::SettingsError SessionController::save()
{
    m_settings.density = m_panel->randomDensity();
    m_settings.speed = m_simulation->speed();
    m_settings.gridVisible = m_board->gridVisible();
    m_settings = shared::normalized(m_settings);

    const QByteArray geometry = m_window != nullptr ? m_window->saveGeometry() : QByteArray();
    QString detail;
    const service::SettingsError error = m_service->save(m_settings, geometry, &detail);
    if (error != service::SettingsError::None)
    {
        qCWarning(lcController) << "session save failed error=" << service::settingsErrorName(error);
        if (m_messages != nullptr)
        {
            m_messages->showMessage(settingsErrorText(error, detail), 8000);
        }
    }
    else
    {
        qCInfo(lcController) << "session saved board=" << m_model->width() << 'x' << m_model->height()
                             << "generation" << m_model->stats().generation;
    }
    return error;
}

void SessionController::applySettings(const shared::AppSettings& values)
{
    m_settings = shared::normalized(values);
    const model::CommandResult result = m_model->setSnapshotInterval(m_settings.snapshotInterval);
    if (model::failed(result) && m_messages != nullptr)
    {
        m_messages->showMessage(tr("设置应用失败：%1").arg(commandResultText(result)), 4000);
    }
    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
    qCInfo(lcController) << "settings applied jump_step=" << m_settings.jumpStep << "size_step="
                         << m_settings.sizeStep << "snapshot_interval=" << m_settings.snapshotInterval;
}

} // namespace lifegame::controller
