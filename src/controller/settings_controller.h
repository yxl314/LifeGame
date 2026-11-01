#pragma once

/**
 * @file settings_controller.h
 * @brief Settings dialog controller.
 */

#include <QObject>

class QMainWindow;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{
class MessageSink;
}

namespace lifegame::controller
{

class SessionController;

/// @brief Settings-dialog input and application.
class SettingsController : public QObject
{
    Q_OBJECT

public:
    /// @param model Model to inspect/apply.
    /// @param session Session owner of current settings.
    /// @param window Dialog parent.
    /// @param messages Status output port.
    /// @param parent QObject parent.
    SettingsController(model::LifeModel* model,
                       SessionController* session,
                       QMainWindow* window,
                       view::MessageSink* messages,
                       QObject* parent = nullptr);

public slots:
    /// @brief Open the settings dialog and save on acceptance.
    void openSettings();

private:
    model::LifeModel* m_model = nullptr;
    SessionController* m_session = nullptr;
    QMainWindow* m_window = nullptr;
    view::MessageSink* m_messages = nullptr;
};

} // namespace lifegame::controller
