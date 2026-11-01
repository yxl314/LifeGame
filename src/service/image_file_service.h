#pragma once

/**
 * @file image_file_service.h
 * @brief PNG image output port and service.
 */

#include <QString>

#include "service/file_error.h"

class QImage;

namespace lifegame::service
{

/// @brief Replaceable PNG output port.
class ImageFilePort
{
public:
    virtual ~ImageFilePort() = default;

    /**
     * @brief Save an image as PNG.
     * @param image Image to encode.
     * @param path Destination path.
     * @param detail Optional encoder error detail.
     * @return FileError code.
     */
    virtual FileError savePng(const QImage& image, const QString& path, QString* detail = nullptr) const = 0;
};

/// @brief QImageWriter-backed PNG service.
class ImageFileService : public ImageFilePort
{
public:
    FileError savePng(const QImage& image, const QString& path, QString* detail = nullptr) const override;
};

} // namespace lifegame::service
