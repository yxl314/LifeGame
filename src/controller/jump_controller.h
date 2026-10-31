#pragma once

#include <QObject>

#include <memory>

#include "model/command_result.h"
#include "view/interfaces.h"

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

/// Rewind/fast-forward input and progress presentation.
class JumpController : public QObject
{
    Q_OBJECT

public:
    JumpController(model::LifeModel* model,
                   view::ControlPanel* panel,
                   view::MessageSink* messages,
                   view::JumpProgressFactory factory,
                   QObject* parent = nullptr);

public slots:
    void rewind();
    void fastForward();
    void jumpBy(int delta);

private slots:
    void rewindBy(int generations);
    void onJumpStarted(const lifegame::shared::JumpInfo& info);
    void onJumpProgress(int done);
    void onJumpFinished(int generation);
    void onJumpCancelled();
    void onJumpFailed(lifegame::model::CommandResult reason);

private:
    void closeProgress();

    model::LifeModel* m_model = nullptr;
    view::ControlPanel* m_panel = nullptr;
    view::MessageSink* m_messages = nullptr;
    view::JumpProgressFactory m_factory;
    std::unique_ptr<view::JumpProgressView> m_progress;
};

} // namespace lifegame::controller
