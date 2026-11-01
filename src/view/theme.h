#pragma once

/**
 * @file theme.h
 * @brief Dark Fusion theme and application icon.
 */

#include <QIcon>

class QApplication;

namespace lifegame::view::theme
{

/// @brief Apply the dark Fusion look. Must run before creating windows.
/// @param app Application instance.
void apply(QApplication& app);

/// @return Programmatically drawn application icon.
QIcon appIcon();

} // namespace lifegame::view::theme
