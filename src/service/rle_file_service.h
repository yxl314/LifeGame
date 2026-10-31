#pragma once

#include <QString>

#include <optional>

#include "domain/rle.h"
#include "service/file_error.h"

namespace lifegame::service
{

struct RleReadResult
{
    std::optional<domain::DecodedDocument> document;
    FileError error = FileError::None;
    domain::RleErrorInfo decodeError;
    QString detail;
};

/// File I/O only; RLE parsing is delegated to the pure domain codec.
class RleFileService
{
public:
    RleReadResult read(const QString& path) const;

    FileError write(const QString& path, const QString& text, QString* detail = nullptr) const;
};

} // namespace lifegame::service
