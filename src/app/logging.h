#pragma once

#include <QLoggingCategory>
#include <QString>

namespace lifegame::app::logging
{

struct Options
{
    bool verbose = false;
    bool mirrorToStderr = true;
    QString filePath; // empty: QStandardPaths::AppLocalDataLocation/logs/life-game.log
};

/// Install the file-backed Qt message handler. Returns false when the file
/// sink cannot be opened (the app still runs with stderr logging).
bool initialize(const Options& options);

/// Restore the previous handler and flush/close the file. Idempotent.
void shutdown();

QString logFilePath();

} // namespace lifegame::app::logging

Q_DECLARE_LOGGING_CATEGORY(lcApp)
