#include "service/image_file_service.h"

#include <QImage>

namespace lifegame::service
{

bool ImageFileService::savePng(const QImage& image, const QString& path) const
{
    return !image.isNull() && image.save(path, "PNG");
}

} // namespace lifegame::service
