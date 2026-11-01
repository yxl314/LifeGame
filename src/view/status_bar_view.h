#pragma once

/**
 * @file status_bar_view.h
 * @brief Status bar implementation of MessageSink.
 */

#include <QObject>

#include "view/interfaces.h"

class QStatusBar;

namespace lifegame::view
{

/// Adapts the narrow MessageSink used by controllers to the window status bar.
class StatusBarView : public QObject, public MessageSink
{
    Q_OBJECT

public:
    /// @param bar Status bar to adapt.
    /// @param parent QObject parent.
    explicit StatusBarView(QStatusBar* bar, QObject* parent = nullptr);

    /// @param text Message.
    /// @param timeoutMs Auto-hide timeout; 0 = sticky.
    void showMessage(const QString& text, int timeoutMs) override;

private:
    QStatusBar* m_bar = nullptr;
};

} // namespace lifegame::view
