#pragma once

#include <QString>

#include "service/file_error.h"

class QImage;

namespace lifegame::service
{

class ImageFileService
{
public:
    FileError savePng(const QImage& image, const QString& path, QString* detail = nullptr) const;
};

} // namespace lifegame::service
