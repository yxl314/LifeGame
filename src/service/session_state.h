#pragma once

#include <QByteArray>
#include <QString>

#include "shared/app_settings.h"

namespace lifegame::service
{

/// Everything persisted between runs.
struct SessionState
{
    shared::AppSettings settings;
    QString boardRle;
    QString ruleNotation = QStringLiteral("B3/S23");
    int boundary = 0; // 0 = dead, 1 = wrap
    int generation = 0;
    QByteArray windowGeometry;
};

} // namespace lifegame::service
