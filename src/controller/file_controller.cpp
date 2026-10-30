#include "controller/file_controller.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>

#include <string>

#include "domain/rle.h"
#include "model/life_model.h"
#include "view/board_view.h"
#include "view/interfaces.h"

namespace lifegame::controller
{

FileController::FileController(model::LifeModel* model,
                               view::BoardView* view,
                               view::MessageSink* messages,
                               QWidget* dialogParent,
                               QObject* parent)
    : QObject(parent),
      m_model(model),
      m_view(view),
      m_messages(messages),
      m_dialogParent(dialogParent),
      m_lastDir(QDir::homePath())
{
}

QString FileController::rleErrorMessage(const domain::RleErrorInfo& error) const
{
    switch (error.code)
    {
    case domain::RleError::None:
        return {};
    case domain::RleError::EmptyBody:
        return tr("文件里没有 RLE 正文。");
    case domain::RleError::InvalidHeader:
        return tr("RLE 头部无法解析。");
    case domain::RleError::InvalidCharacter:
        return tr("无法识别的 RLE 字符（位置 %1）。").arg(error.position + 1);
    case domain::RleError::RunTooLong:
        return tr("RLE 游程长度异常。");
    case domain::RleError::TooLarge:
        return tr("图案尺寸超出棋盘上限（%1 × %1）。").arg(domain::kMaxBoardExtent);
    case domain::RleError::InvalidRule:
        return tr("RLE 头部规则记号无效。");
    }
    return {};
}

void FileController::openRle()
{
    const QString path = QFileDialog::getOpenFileName(
        m_dialogParent,
        tr("打开 RLE 图案"),
        m_lastDir,
        tr("生命游戏图案 (*.rle *.RLE *.txt);;所有文件 (*)"));
    if (path.isEmpty())
    {
        return;
    }

    const service::RleReadResult result = m_rle.read(path);
    if (!result.systemError.isEmpty())
    {
        QMessageBox::warning(m_dialogParent, tr("打开失败"), tr("无法读取文件：%1").arg(result.systemError));
        return;
    }
    if (result.decodeError.code != domain::RleError::None || !result.document.has_value())
    {
        QMessageBox::warning(m_dialogParent, tr("打开失败"), rleErrorMessage(result.decodeError));
        return;
    }
    if (!m_model->loadDocument(*result.document))
    {
        QMessageBox::warning(m_dialogParent,
                             tr("打开失败"),
                             tr("无法载入图案：跳转进行中、尺寸超限或规则不受支持。"));
        return;
    }

    m_lastDir = QFileInfo(path).absolutePath();
    if (m_messages != nullptr)
    {
        m_messages->showMessage(tr("已打开 %1（%2 × %3）。")
                                    .arg(QFileInfo(path).fileName())
                                    .arg(m_model->width())
                                    .arg(m_model->height()),
                                6000);
    }
}

void FileController::saveRle()
{
    QString path = QFileDialog::getSaveFileName(
        m_dialogParent,
        tr("保存为 RLE"),
        m_lastDir + QStringLiteral("/life-game.rle"),
        tr("生命游戏图案 (*.rle);;所有文件 (*)"));
    if (path.isEmpty())
    {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty())
    {
        path += QStringLiteral(".rle");
    }

    QString error;
    const QString text = QString::fromStdString(m_model->toRle("life-game"));
    if (!m_rle.write(path, text, &error))
    {
        QMessageBox::warning(m_dialogParent, tr("保存失败"), tr("无法写入文件：%1").arg(error));
        return;
    }

    m_lastDir = QFileInfo(path).absolutePath();
    if (m_messages != nullptr)
    {
        m_messages->showMessage(tr("已保存到 %1。").arg(QFileInfo(path).fileName()), 6000);
    }
}

void FileController::exportImage()
{
    QString path = QFileDialog::getSaveFileName(
        m_dialogParent,
        tr("导出 PNG 图片"),
        m_lastDir + QStringLiteral("/life-game.png"),
        tr("PNG 图片 (*.png);;所有文件 (*)"));
    if (path.isEmpty())
    {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty())
    {
        path += QStringLiteral(".png");
    }

    const QImage image = m_view->renderBoardImage();
    if (!m_image.savePng(image, path))
    {
        QMessageBox::warning(m_dialogParent, tr("导出失败"), tr("无法写入图片文件。"));
        return;
    }

    m_lastDir = QFileInfo(path).absolutePath();
    if (m_messages != nullptr)
    {
        m_messages->showMessage(tr("已导出图片 %1。").arg(QFileInfo(path).fileName()), 6000);
    }
}

} // namespace lifegame::controller
