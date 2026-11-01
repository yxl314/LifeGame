#pragma once

#include <QObject>

#include "model/stall_reason.h"
#include "shared/app_settings.h"

class QTimer;
class QAction;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{
class ControlPanel;
class MessageSink;
}

namespace lifegame::controller
{

/// Simulation clock and play/pause/step/speed intents.
class SimulationController : public QObject
{
    Q_OBJECT

public:
    SimulationController(model::LifeModel* model,
                         view::ControlPanel* panel,
                         view::MessageSink* messages,
                         QObject* parent = nullptr);

    int speed() const;

    void setPlayPauseAction(QAction* action);

    /// Used when restoring the session; never called while the slider is being dragged.
    void setSpeedExternal(int speed);

signals:
    void speedChanged(int speed);

public slots:
    void toggleRunning();
    void stepOnce();

private slots:
    void onTick();
    void onRunningChanged(bool running);
    void onSpeedChanged(int speed);
    void onStalled(lifegame::model::StallReason reason);

private:
    void applySpeed(int speed);
    void restartTimer();
    void updatePlayAction();

    model::LifeModel* m_model = nullptr;
    view::ControlPanel* m_panel = nullptr;
    view::MessageSink* m_messages = nullptr;
    QTimer* m_timer = nullptr;
    int m_speed = shared::limits::kDefaultSpeed;
    QAction* m_playAction = nullptr;
};

} // namespace lifegame::controller
