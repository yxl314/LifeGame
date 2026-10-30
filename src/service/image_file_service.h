#pragma once

#include <QString>

class QImage;

namespace lifegame::service
{

class ImageFileService
{
public:
    bool savePng(const QImage& image, const QString& path) const;
};

} // namespace lifegame::service
