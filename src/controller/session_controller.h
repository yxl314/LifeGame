#pragma once

#include <QObject>

#include "shared/app_settings.h"
#include "service/settings_error.h"

#include <memory>

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
class SessionService;
class SimulationController;

/// Application/session coordinator: restore on start, persist on exit.
/// The SettingsController owns the dialog; file services live elsewhere.
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
    ~SessionController() override;

    /// Returns true when a persisted board was restored.
    bool restore();

    service::SettingsError save();

    const shared::AppSettings& settings() const;

    /// Apply the three editable settings fields (snapshot/jump/size).
    void applySettings(const shared::AppSettings& values);

    model::LifeModel* m_model = nullptr;
    view::ControlPanel* m_panel = nullptr;
    SimulationController* m_simulation = nullptr;
    BoardController* m_board = nullptr;
    std::unique_ptr<SessionService> m_service;
    QMainWindow* m_window = nullptr;
    view::MessageSink* m_messages = nullptr;
    shared::AppSettings m_settings;
};

} // namespace lifegame::controller
