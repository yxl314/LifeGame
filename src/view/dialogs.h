#pragma once

/**
 * @file dialogs.h
 * @brief Localized message-box helpers.
 */

#include <QString>

class QWidget;

namespace lifegame::view
{

/// @brief Show a Chinese-labeled warning box.
/// @param parent Dialog parent.
/// @param title Dialog title.
/// @param text Warning text.
void showWarning(QWidget* parent, const QString& title, const QString& text);

} // namespace lifegame::view
