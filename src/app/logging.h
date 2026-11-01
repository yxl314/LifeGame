#pragma once

/**
 * @file logging.h
 * @brief File-backed Qt message handler and logging options.
 */

#include <QLoggingCategory>
#include <QString>

namespace lifegame::app::logging
{

/// @brief Logging sink and filtering options.
struct Options
{
    bool verbose = false;        ///< Enable lifegame.* debug logging.
    bool mirrorToStderr = true;  ///< Mirror accepted messages to stderr.
    QString filePath;            ///< Empty = default AppLocalDataLocation path.
};

/**
 * @brief Install the file-backed Qt message handler.
 * @param options Sink/filter options.
 * @return False when the file sink cannot be opened; stderr logging still works.
 */
bool initialize(const Options& options);

/// @brief Restore the previous handler and flush/close the file. Idempotent.
void shutdown();

/// @return Active log file path, or empty before initialize().
QString logFilePath();

} // namespace lifegame::app::logging

Q_DECLARE_LOGGING_CATEGORY(lcApp)
