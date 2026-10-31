#pragma once

#include <QString>

class QWidget;

namespace lifegame::view
{

/// Chinese-labeled warning box; QMessageBox::warning's standard button text
/// is English unless a Qt translation is installed.
void showWarning(QWidget* parent, const QString& title, const QString& text);

} // namespace lifegame::view
