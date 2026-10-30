#include "service/rle_file_service.h"

#include <QFile>
#include <QIODevice>

#include <string_view>

namespace lifegame::service
{

RleReadResult RleFileService::read(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {std::nullopt, file.errorString(), {}};
    }

    const QByteArray data = file.readAll();
    const domain::RleDecodeResult decoded =
        domain::decodeRle(std::string_view(data.constData(), static_cast<std::size_t>(data.size())));
    if (!decoded.document.has_value())
    {
        return {std::nullopt, QString(), decoded.error};
    }
    return {decoded.document, QString(), {}};
}

bool RleFileService::write(const QString& path, const QString& text, QString* error) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }

    const QByteArray data = text.toUtf8();
    if (file.write(data) != data.size() || !file.flush())
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

} // namespace lifegame::service
