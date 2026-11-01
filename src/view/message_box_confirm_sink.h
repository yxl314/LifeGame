#pragma once

/**
 * @file message_box_confirm_sink.h
 * @brief QMessageBox confirmation sink.
 */

#include "view/interfaces.h"

class QWidget;

namespace lifegame::view
{

/// QMessageBox-backed ConfirmSink for the real application.
class MessageBoxConfirmSink : public ConfirmSink
{
public:
    /// @param parent Dialog parent.
    explicit MessageBoxConfirmSink(QWidget* parent = nullptr);

    /// @param title Dialog title.
    /// @param text Confirmation body.
    /// @return True when user accepted.
    bool confirm(const QString& title, const QString& text) override;

private:
    QWidget* m_parent = nullptr;
};

} // namespace lifegame::view
