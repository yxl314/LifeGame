#include "controller/session_controller.h"

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

const model::AppSettings& SessionController::settings() const
{
    return m_settings;
}

bool SessionController::restore()
{
    const service::SessionState state = m_store->load();
    m_settings = model::normalized(state.settings);

    m_model->setSnapshotInterval(m_settings.snapshotInterval);
    m_model->setBoundary(state.boundary == 1 ? domain::Boundary::Wrap : domain::Boundary::Dead);
    m_model->setRuleByNotation(state.ruleNotation.toStdString());

    bool restored = false;
    if (!state.boardRle.isEmpty())
    {
        const domain::RleDecodeResult decoded = domain::decodeRle(state.boardRle.toStdString());
        if (decoded.document.has_value() && m_model->loadDocument(*decoded.document))
        {
            restored = true;
            if (state.generation > 0)
            {
                m_model->setGeneration(state.generation);
            }
        }
    }

    if (!restored)
    {
        m_model->resizeBoard(domain::kDefaultBoardWidth, domain::kDefaultBoardHeight);
    }

    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
    m_panel->setRandomDensity(m_settings.density);
    m_panel->setSpeed(m_settings.speed);
    m_simulation->setSpeedExternal(m_settings.speed);
    m_board->setGridVisible(m_settings.gridVisible);

    if (m_window != nullptr && !state.windowGeometry.isEmpty())
    {
        m_window->restoreGeometry(state.windowGeometry);
    }

    return restored;
}

void SessionController::save()
{
    m_settings.density = m_panel->randomDensity();
    m_settings.speed = m_simulation->speed();
    m_settings.gridVisible = m_board->gridVisible();
    m_settings = model::normalized(m_settings);

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
    m_store->save(state);
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
        return;
    }

    const model::AppSettings edited = dialog.values();
    m_settings.jumpStep = edited.jumpStep;
    m_settings.sizeStep = edited.sizeStep;
    m_settings.snapshotInterval = edited.snapshotInterval;
    m_settings = model::normalized(m_settings);

    applySettings();
    save();

    if (m_messages != nullptr)
    {
        m_messages->showMessage(tr("设置已保存。"), 3000);
    }
}

void SessionController::applySettings()
{
    m_model->setSnapshotInterval(m_settings.snapshotInterval);
    m_panel->setJumpStep(m_settings.jumpStep);
    m_panel->setSizeStep(m_settings.sizeStep);
}

} // namespace lifegame::controller
