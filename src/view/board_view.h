#pragma once

#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QRectF>
#include <QVector>
#include <QWidget>

namespace lifegame::domain
{
class Grid;
}

namespace lifegame::view
{

/**
 * Custom-painted board view.
 *
 * Layered MVC passive view: the controller pushes a `const domain::Grid*`
 * plus a little render state. The view never includes or observes the Model,
 * never copies the board and never calls a mutating model method.
 */
class BoardView : public QWidget
{
    Q_OBJECT

public:
    explicit BoardView(QWidget* parent = nullptr);

    /// Passive-view API: the controller owns the model and pushes a read-only
    /// board pointer plus the small amount of state this view renders.
    void setBoard(const domain::Grid* board);
    void setPopulation(int population);
    void setRunning(bool running);
    void refresh();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool gridVisible() const;
    QImage renderBoardImage() const;

public slots:
    void setStampPreview(const QVector<QPoint>& offsets);
    void setGridVisible(bool visible);
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void cellPressed(int row, int col);
    void cellDragged(int row, int col);
    void cellReleased(int row, int col);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int boardWidth() const;
    int boardHeight() const;

    double cellPixels() const;
    QPointF boardTopLeft() const;
    double fitCellPixels() const;
    bool cellFromPosition(const QPoint& position, int* row, int* col) const;
    void clampPan();
    void zoomBy(double factor);

    void drawBoard(QPainter& painter,
                   const domain::Grid& board,
                   const QPointF& topLeft,
                   double cellPixels,
                   bool interactive) const;
    void drawCellsDetailed(QPainter& painter,
                           const domain::Grid& board,
                           const QPointF& topLeft,
                           double cellPixels,
                           double gap,
                           double radius,
                           bool interactive) const;
    void drawCellsAsImage(QPainter& painter,
                          const domain::Grid& board,
                          const QRectF& boardRect,
                          bool interactive) const;
    void updateGridPixmap(const QRectF& boardRect, double cellPixels) const;
    void drawGridLines(QPainter& painter, const QRectF& boardRect, double cellPixels) const;

    const domain::Grid* m_board = nullptr;
    int m_population = 0;

    QVector<QPoint> m_preview;
    bool m_grid_visible = true;
    bool m_animating = false;

    mutable QImage m_cell_image;
    mutable QPixmap m_grid_pixmap;
    mutable QRectF m_grid_key;
    mutable double m_grid_cell = 0.0;
    mutable qreal m_grid_dpr = 0.0;

    double m_cell_pixels = 8.0;
    bool m_auto_fit = true;
    QPointF m_pan;

    int m_hover_row = -1;
    int m_hover_col = -1;
    bool m_left_pressed = false;
    bool m_panning = false;
    QPoint m_last_mouse;
};

} // namespace lifegame::view
