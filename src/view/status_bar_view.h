#pragma once

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
    explicit StatusBarView(QStatusBar* bar, QObject* parent = nullptr);

    void showMessage(const QString& text, int timeoutMs) override;

private:
    QStatusBar* m_bar = nullptr;
};

} // namespace lifegame::view
