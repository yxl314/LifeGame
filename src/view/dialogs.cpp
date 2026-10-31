#include "view/dialogs.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>

namespace lifegame::view
{

void showWarning(QWidget* parent, const QString& title, const QString& text)
{
    QMessageBox box(QMessageBox::Warning, title, text, QMessageBox::NoButton, parent);
    QPushButton* close = box.addButton(QCoreApplication::translate("Dialogs", "确定"),
                                       QMessageBox::AcceptRole);
    close->setIcon(QIcon(QStringLiteral(":/icons/check.svg")));
    box.exec();
}

} // namespace lifegame::view
