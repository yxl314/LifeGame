#include "controller/session_controller.h"

#include "controller/controller_log.h"
#include "controller/error_text.h"

#include <QDialog>
#include <QMainWindow>

#include "controller/board_controller.h"
#include "controller/simulation_controller.h"
#include "domain/rle.h"
#include "model/life_model.h"
#include "service/session_state.h"
#include "service/settings_store.h"
#include "view/control_panel.h"
#include "view/interfaces.h"
#include "view/settings_dialog.h"

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
      m_store(store),
      m_window(window),
      m_messages(messages)
{
}

const shared::AppSettings& SessionController::settings() const
{
    return m_settings;
}

bool SessionController::restore()
{
    const service::SettingsLoadResult loadedState = m_store->load();
    const service::SessionState state = loadedState.state;
    m_settings = shared::normalized(state.settings);

    qCInfo(lcController) << "session restore start has_board=" << !state.boardRle.isEmpty()
                         << "generation=" << state.generation << "rule=" << state.ruleNotation
                         << "boundary=" << state.boundary;
    if (loadedState.error != service::SettingsError::None)
    {
        qCWarning(lcController) << "settings restore failed error="
                                << service::settingsErrorName(loadedState.error);
        if (m_messages != nullptr)
        {
            m_messages->showMessage(settingsErrorText(loadedState.error, loadedState.detail), 8000);
        }
    }

    m_model->setSnapshotInterval(m_settings.snapshotInterval);
    m_model->setBoundary(state.boundary == 1 ? domain::Boundary::Wrap : domain::Boundary::Dead);
    m_model->setRuleByNotation(state.ruleNotation.toStdString());

    bool restored = false;
    if (!state.boardRle.isEmpty())
    {
        const domain::RleDecodeResult decoded = domain::decodeRle(state.boardRle.toStdString());
        if (decoded.document.has_value())
        {
            const model::CommandResult loaded = m_model->loadDocument(*decoded.document);
            if (model::succeeded(loaded))
            {
                restored = true;
                if (state.generation > 0)
                {
                    (void)m_model->setGeneration(state.generation);
                }
            }
            else if (m_messages != nullptr)
            {
                m_messages->showMessage(
                    tr("上次会话的棋盘无法恢复：%1 已改为示例棋盘。").arg(commandResultText(loaded)), 8000);
            }
        }
        else
        {
            qCWarning(lcController) << "stored RLE decode failed code="
                                    << domain::rleErrorName(decoded.error.code) << "line="
                                    << decoded.error.line << "column=" << decoded.error.column
                                    << "length=" << state.boardRle.size();
            if (m_messages != nullptr)
            {
                m_messages->showMessage(tr("上次会话的棋盘无法解析，已改为示例棋盘。"), 8000);
            }
        }
    }

    if (!restored)
    {
        const model::CommandResult resized =
            m_model->resizeBoard(domain::kDefaultBoardWidth, domain::kDefaultBoardHeight);
        if (model::failed(resized) && m_messages != nullptr)
        {
            m_messages->showMessage(commandResultText(resized), 5000);
        }
    }

    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
    m_panel->setRandomDensity(m_settings.density);
    m_panel->setSpeed(m_settings.speed);
    m_simulation->setSpeedExternal(m_settings.speed);
    m_board->setGridVisible(m_settings.gridVisible);

    if (m_window != nullptr && !state.windowGeometry.isEmpty())
    {
        if (!m_window->restoreGeometry(state.windowGeometry))
        {
            qCDebug(lcController) << "window geometry restore failed";
        }
    }

    qCInfo(lcController) << "session restore done restored=" << restored << "board=" << m_model->width() << 'x'
                         << m_model->height() << "generation" << m_model->stats().generation;
    return restored;
}

service::SettingsError SessionController::save()
{
    m_settings.density = m_panel->randomDensity();
    m_settings.speed = m_simulation->speed();
    m_settings.gridVisible = m_board->gridVisible();
    m_settings = shared::normalized(m_settings);

    service::SessionState state;
    state.settings = m_settings;
    state.boardRle = QString::fromStdString(m_model->toRle("life-game"));
    state.ruleNotation = QString::fromStdString(m_model->rule().notation());
    state.boundary = m_model->boundary() == domain::Boundary::Wrap ? 1 : 0;
    state.generation = m_model->stats().generation;
    if (m_window != nullptr)
    {
        state.windowGeometry = m_window->saveGeometry();
    }

    QString detail;
    const service::SettingsError error = m_store->save(state, &detail);
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

void SessionController::openSettings()
{
    if (m_model->isJumping())
    {
        if (m_messages != nullptr)
        {
            m_messages->showMessage(tr("跳转进行中，请稍后再修改设置。"), 3000);
        }
        return;
    }

    view::SettingsDialog dialog(m_window);
    dialog.setValues(m_settings);
    if (dialog.exec() != QDialog::Accepted)
    {
        qCDebug(lcController) << "settings dialog cancelled";
        return;
    }

    m_settings = shared::normalized(dialog.values());

    applySettings();
    save();

    qCInfo(lcController) << "settings applied jump_step=" << m_settings.jumpStep << "size_step="
                         << m_settings.sizeStep << "snapshot_interval=" << m_settings.snapshotInterval;
    if (m_messages != nullptr)
    {
        m_messages->showMessage(tr("设置已保存。"), 3000);
    }
}

void SessionController::applySettings()
{
    const model::CommandResult result = m_model->setSnapshotInterval(m_settings.snapshotInterval);
    if (model::failed(result) && m_messages != nullptr)
    {
        m_messages->showMessage(tr("设置应用失败：%1").arg(commandResultText(result)), 4000);
    }
    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
}

} // namespace lifegame::controller
