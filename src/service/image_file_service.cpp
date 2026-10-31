#include "service/image_file_service.h"

#include "service/service_log.h"

#include <QImage>
#include <QImageWriter>

namespace lifegame::service
{

FileError ImageFileService::savePng(const QImage& image, const QString& path, QString* detail) const
{
    if (image.isNull())
    {
        return FileError::NullImage;
    }

    QImageWriter writer(path, "png");
    if (!writer.write(image))
    {
        if (detail != nullptr)
        {
            *detail = writer.errorString();
        }
        qCWarning(lcIo) << "PNG write failed path=" << path << "error=" << writer.errorString();
        return FileError::EncodeFailed;
    }
    qCInfo(lcIo) << "PNG written path=" << path << "size=" << image.width() << 'x' << image.height();
    return FileError::None;
}

} // namespace lifegame::service
