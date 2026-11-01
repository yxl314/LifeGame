#pragma once

/**
 * @file session_state.h
 * @brief Persisted session value object.
 */

#include <QByteArray>
#include <QString>

#include "shared/app_settings.h"

namespace lifegame::service
{

/// @brief Everything persisted between runs.
struct SessionState
{
    shared::AppSettings settings;                    ///< User settings.
    QString boardRle;                                ///< Encoded board.
    QString ruleNotation = QStringLiteral("B3/S23"); ///< Canonical rule.
    int boundary = 0;                                ///< 0 = dead, 1 = wrap.
    int generation = 0;                              ///< Current generation.
    QByteArray windowGeometry;                       ///< Saved window geometry.
};

} // namespace lifegame::service
