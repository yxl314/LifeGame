#pragma once

#include <QIcon>

class QApplication;

namespace lifegame::view::theme
{

/// Apply the dark Fusion look. Must be called before creating windows.
void apply(QApplication& app);

/// Programmatically drawn application icon (no external asset to forget).
QIcon appIcon();

} // namespace lifegame::view::theme
