#pragma once

#include <QMetaObject>
#include <QObject>

#include "shared/app_settings.h"

class QTimer;
class QScreen;
class QWindow;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{
class BoardView;
class ControlPanel;
}

namespace lifegame::controller
{

/**
 * Pushes Model state into the passive views and owns the frame scheduler.
 *
 * BoardController handles input/commands only; this presenter owns the
 * Model->View direction and the decision to coalesce repaints.
 */
class BoardPresenter : public QObject
{
    Q_OBJECT

public:
    BoardPresenter(model::LifeModel* model,
                   view::BoardView* view,
                   view::ControlPanel* panel,
                   QObject* parent = nullptr);

    /// Sync with the current model state, bypassing the frame timer.
    void sync();

    /// Set the current generation rate used for the frame-budget decision.
    void setGenerationRate(int generationsPerSecond);

private slots:
    void requestUpdate();
    void updateNow();

private:
    void watchWindowScreen();
    void refreshScreenRate();
    bool needsCoalescing() const;

    model::LifeModel* m_model = nullptr;
    view::BoardView* m_view = nullptr;
    view::ControlPanel* m_panel = nullptr;
    QTimer* m_timer = nullptr;
    int m_screenRefresh = 60;
    int m_generationRate = shared::limits::kDefaultSpeed;
    QMetaObject::Connection m_screenConnection;
    QMetaObject::Connection m_windowScreenConnection;
};

} // namespace lifegame::controller
