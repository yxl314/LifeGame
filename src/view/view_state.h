#pragma once

#include <QString>

#include "shared/life_stats.h"

namespace lifegame::domain
{
class Grid;
}

namespace lifegame::view
{

/// One complete board frame. Controllers apply this atomically instead of
/// calling setBoard/setPopulation/setRunning separately.
struct BoardFrame
{
    const domain::Grid* board = nullptr;
    int population = 0;
    bool running = false;
};

/// One complete side-panel state.
/// Half-open visible cell range used by the painter.
struct VisibleCellRange
{
    int firstRow = 0;
    int lastRow = 0;
    int firstCol = 0;
    int lastCol = 0;
};

struct PanelState
{
    shared::LifeStats stats;
    int width = 0;
    int height = 0;
    QString ruleNotation = QStringLiteral("B3/S23");
    int boundaryIndex = 0;
    int oldestGeneration = 0;
    bool running = false;
};

} // namespace lifegame::view
