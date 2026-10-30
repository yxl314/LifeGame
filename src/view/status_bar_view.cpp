#include "view/status_bar_view.h"

#include <QStatusBar>

namespace lifegame::view
{

StatusBarView::StatusBarView(QStatusBar* bar, QObject* parent)
    : QObject(parent), m_bar(bar)
{
}

void StatusBarView::showMessage(const QString& text, int timeoutMs)
{
    if (m_bar != nullptr)
    {
        m_bar->showMessage(text, timeoutMs);
    }
}

} // namespace lifegame::view
