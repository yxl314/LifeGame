#include "controller/error_text.h"

#include <QCoreApplication>

namespace lifegame::controller
{

QString commandResultText(model::CommandResult result)
{
    const auto text = [](const char* source) {
        return QCoreApplication::translate("CommandError", source);
    };

    switch (result)
    {
    case model::CommandResult::Success:
        return {};
    case model::CommandResult::Busy:
        return text("跳转正在进行，请等它结束后再试。");
    case model::CommandResult::OutOfBounds:
        return text("目标位置超出棋盘范围。");
    case model::CommandResult::NoChange:
        return text("操作没有改变任何状态。");
    case model::CommandResult::InvalidRule:
        return text("规则记号无效。");
    case model::CommandResult::InvalidPattern:
        return text("图案无效或为空。");
    case model::CommandResult::InvalidArgument:
        return text("参数无效。");
    case model::CommandResult::InvalidTarget:
        return text("跳转目标无效：必须是非负且不同于当前的代数。");
    case model::CommandResult::NoHistory:
        return text("没有可用的历史快照，无法回退到该代。");
    case model::CommandResult::EmptyBoard:
        return text("棋盘为空，请先绘制细胞或使用随机填充。");
    case model::CommandResult::NotRunning:
        return text("模拟当前未运行。");
    case model::CommandResult::Clipped:
        return text("图案超出棋盘边缘，已裁掉越界部分。");
    case model::CommandResult::Adjusted:
        return text("数值已调整到允许范围。");
    case model::CommandResult::TooLarge:
        return text("尺寸超出棋盘上限。");
    case model::CommandResult::OutOfMemory:
        return text("内存不足，操作未完成。");
    case model::CommandResult::ComputationFailed:
        return text("后台计算失败，模拟已停止。");
    }
    return text("未知错误。");
}

QString fileErrorText(service::FileError error, const QString& detail)
{
    const auto text = [](const char* source) {
        return QCoreApplication::translate("FileError", source);
    };

    QString message;
    switch (error)
    {
    case service::FileError::None:
        return {};
    case service::FileError::OpenFailed:
        message = text("无法打开文件。");
        break;
    case service::FileError::ReadFailed:
        message = text("读取文件失败。");
        break;
    case service::FileError::WriteFailed:
        message = text("写入文件失败。");
        break;
    case service::FileError::DecodeFailed:
        message = text("文件内容不是有效的 RLE。");
        break;
    case service::FileError::EncodeFailed:
        message = text("PNG 编码失败。");
        break;
    case service::FileError::NullImage:
        message = text("图像为空，无法导出。");
        break;
    case service::FileError::TooLarge:
        message = text("文件过大，已拒绝读取。");
        break;
    }
    if (!detail.isEmpty())
    {
        message += QStringLiteral("（%1）").arg(detail);
    }
    return message;
}

QString settingsErrorText(service::SettingsError error, const QString& detail)
{
    const auto text = [](const char* source) {
        return QCoreApplication::translate("SettingsError", source);
    };

    QString message;
    switch (error)
    {
    case service::SettingsError::None:
        return {};
    case service::SettingsError::AccessError:
        message = text("无法访问配置文件。");
        break;
    case service::SettingsError::FormatError:
        message = text("配置文件格式损坏。");
        break;
    case service::SettingsError::CorruptData:
        message = text("配置文件包含无效值，已使用默认设置。");
        break;
    }
    if (!detail.isEmpty())
    {
        message += QStringLiteral("（%1）").arg(detail);
    }
    return message;
}

} // namespace lifegame::controller
