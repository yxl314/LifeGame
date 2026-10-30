#pragma once

#include <QObject>
#include <QVector>

#include "domain/pattern.h"

class QAction;
class QPoint;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{
class BoardView;
class ControlPanel;
class MessageSink;
class PatternCatalog;
} // namespace view

namespace lifegame::controller
{

/// Mouse input for the board: painting, stamping, clear/random/demo, grid.
class BoardController : public QObject
{
    Q_OBJECT

public:
    BoardController(model::LifeModel* model,
                    view::BoardView* view,
                    view::ControlPanel* panel,
                    const view::PatternCatalog* catalog,
                    view::MessageSink* messages,
                    QObject* parent = nullptr);

    void setGridAction(QAction* action);
    bool gridVisible() const;

public slots:
    void setGridVisible(bool visible);
    void clearBoard();
    void randomizeBoard();
    void loadDemo();
    void selectPattern(int patternId);

private slots:
    void onCellPressed(int row, int col);
    void onCellDragged(int row, int col);
    void onCellReleased(int row, int col);

private:
    void paintLine(int fromRow, int fromCol, int toRow, int toCol, bool alive);
    void applyStamp(int row, int col);

    model::LifeModel* m_model = nullptr;
    view::BoardView* m_view = nullptr;
    view::ControlPanel* m_panel = nullptr;
    const view::PatternCatalog* m_catalog = nullptr;
    view::MessageSink* m_messages = nullptr;

    bool m_painting = false;
    bool m_paintValue = false;
    int m_lastRow = -1;
    int m_lastCol = -1;

    bool m_stampMode = false;
    domain::Pattern m_stamp;

    bool m_gridVisible = true;
    QAction* m_gridAction = nullptr;
};

} // namespace lifegame::controller
