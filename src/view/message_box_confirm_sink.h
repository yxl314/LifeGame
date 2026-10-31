#pragma once

#include "view/interfaces.h"

class QWidget;

namespace lifegame::view
{

/// QMessageBox-backed ConfirmSink for the real application.
class MessageBoxConfirmSink : public ConfirmSink
{
public:
    explicit MessageBoxConfirmSink(QWidget* parent = nullptr);

    bool confirm(const QString& title, const QString& text) override;

private:
    QWidget* m_parent = nullptr;
};

} // namespace lifegame::view
