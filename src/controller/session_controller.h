#pragma once

#include <QObject>

#include "model/app_settings.h"
#include "service/settings_error.h"

class QMainWindow;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::service
{
class SettingsStore;
}

namespace lifegame::view
{
class ControlPanel;
class MessageSink;
}

namespace lifegame::controller
{

class BoardController;
class SimulationController;

/// Application/session coordinator: restore on start, persist on exit,
/// apply the settings dialog. This is the only controller that knows services.
class SessionController : public QObject
{
    Q_OBJECT

public:
    SessionController(model::LifeModel* model,
                      view::ControlPanel* panel,
                      SimulationController* simulation,
                      BoardController* board,
                      service::SettingsStore* store,
                      QMainWindow* window,
                      view::MessageSink* messages,
                      QObject* parent = nullptr);

    /// Returns true when a persisted board was restored.
    bool restore();

    service::SettingsError save();

    const model::AppSettings& settings() const;

public slots:
    void openSettings();

private:
    void applySettings();

    model::LifeModel* m_model = nullptr;
    view::ControlPanel* m_panel = nullptr;
    SimulationController* m_simulation = nullptr;
    BoardController* m_board = nullptr;
    service::SettingsStore* m_store = nullptr;
    QMainWindow* m_window = nullptr;
    view::MessageSink* m_messages = nullptr;
    model::AppSettings m_settings;
};

} // namespace lifegame::controller
