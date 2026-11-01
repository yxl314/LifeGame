#pragma once

#include <QString>

#include "service/file_error.h"

class QImage;

namespace lifegame::service
{

class ImageFilePort
{
public:
    virtual ~ImageFilePort() = default;
    virtual FileError savePng(const QImage& image, const QString& path, QString* detail = nullptr) const = 0;
};

class ImageFileService : public ImageFilePort
{
public:
    FileError savePng(const QImage& image, const QString& path, QString* detail = nullptr) const override;
};

} // namespace lifegame::service
