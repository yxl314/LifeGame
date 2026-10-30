#include "view/jump_progress_dialog.h"

#include <QCoreApplication>
#include <QObject>
#include <QProgressDialog>

#include <cstdlib>

namespace lifegame::view
{

namespace
{

QString msg(const char* text)
{
    return QCoreApplication::translate("JumpProgressDialog", text);
}

} // namespace

JumpProgressDialog::JumpProgressDialog(QWidget* parent)
    : m_parent(parent)
{
}

JumpProgressDialog::~JumpProgressDialog()
{
    closeDialog();
}

void JumpProgressDialog::setCancelHandler(std::function<void()> handler)
{
    m_cancelHandler = std::move(handler);
}

void JumpProgressDialog::show(const model::JumpInfo& info)
{
    close();

    const int distance = std::abs(info.toGeneration - info.fromGeneration);
    const bool forward = info.toGeneration > info.fromGeneration;
    QString text = forward ? msg("正在快进 %1 代…").arg(distance)
                           : msg("正在回退 %1 代…").arg(distance);
    if (info.replayGenerations > distance)
    {
        text += msg("\n（需从起点重放 %1 代）").arg(info.replayGenerations);
    }

    m_dialog = std::make_unique<QProgressDialog>(text, msg("取消"), 0, std::max(1, info.replayGenerations), m_parent);
    m_dialog->setWindowTitle(msg("生命游戏"));
    m_dialog->setWindowModality(Qt::WindowModal);
    m_dialog->setMinimumDuration(300);
    m_dialog->setAutoClose(false);
    m_dialog->setAutoReset(false);
    m_dialog->setValue(0);

    auto cancel = [this] {
        if (m_cancelHandler)
        {
            m_cancelHandler();
        }
    };
    QObject::connect(m_dialog.get(), &QProgressDialog::canceled, m_dialog.get(), cancel);
    QObject::connect(m_dialog.get(), &QProgressDialog::rejected, m_dialog.get(), cancel);
}

void JumpProgressDialog::setProgress(int done)
{
    if (m_dialog)
    {
        m_dialog->setValue(done);
    }
}

void JumpProgressDialog::close()
{
    closeDialog();
}

void JumpProgressDialog::closeDialog()
{
    if (!m_dialog)
    {
        return;
    }
    m_dialog->close();
    m_dialog.reset();
}

} // namespace lifegame::view
