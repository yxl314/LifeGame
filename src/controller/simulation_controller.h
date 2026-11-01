#pragma once

/**
 * @file simulation_controller.h
 * @brief Simulation clock and play/pause/step/speed controller.
 */

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

/// @brief Simulation clock and play/pause/step/speed intents.
class SimulationController : public QObject
{
    Q_OBJECT

public:
    /// @param model Model to drive.
    /// @param panel Speed/play UI.
    /// @param messages Status output port.
    /// @param parent QObject parent.
    SimulationController(model::LifeModel* model,
                         view::ControlPanel* panel,
                         view::MessageSink* messages,
                         QObject* parent = nullptr);

    /// @return Current generations per second (0 = pause).
    int speed() const;

    /// @param action Menu/toolbar play-pause action to keep in sync.
    void setPlayPauseAction(QAction* action);

    /// @param speed Restored speed; pauses when 0.
    void setSpeedExternal(int speed);

signals:
    /// @param speed New generations per second.
    void speedChanged(int speed);

public slots:
    /// @brief Toggle running, honoring the zero-speed pause rule.
    void toggleRunning();
    /// @brief Pause and evolve one generation synchronously.
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
