#pragma once

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

/// Settings-dialog input and application; separate from session persistence.
class SettingsController : public QObject
{
    Q_OBJECT

public:
    SettingsController(model::LifeModel* model,
                       SessionController* session,
                       QMainWindow* window,
                       view::MessageSink* messages,
                       QObject* parent = nullptr);

public slots:
    void openSettings();

private:
    model::LifeModel* m_model = nullptr;
    SessionController* m_session = nullptr;
    QMainWindow* m_window = nullptr;
    view::MessageSink* m_messages = nullptr;
};

} // namespace lifegame::controller
