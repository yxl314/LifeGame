#include "view/message_box_confirm_sink.h"

#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>

namespace lifegame::view
{

MessageBoxConfirmSink::MessageBoxConfirmSink(QWidget* parent)
    : m_parent(parent)
{
}

bool MessageBoxConfirmSink::confirm(const QString& title, const QString& text)
{
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, m_parent);
    QPushButton* accept = box.addButton(QCoreApplication::translate("Confirm", "确认"),
                                        QMessageBox::AcceptRole);
    QPushButton* cancel = box.addButton(QCoreApplication::translate("Confirm", "取消"),
                                        QMessageBox::RejectRole);
    accept->setIcon(QIcon(QStringLiteral(":/icons/check.svg")));
    cancel->setIcon(QIcon(QStringLiteral(":/icons/x.svg")));
    box.setDefaultButton(cancel);
    box.exec();
    return box.clickedButton() == accept;
}

} // namespace lifegame::view
