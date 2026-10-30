#pragma once

#include <QString>

#include <optional>

#include "domain/rle.h"

namespace lifegame::service
{

struct RleReadResult
{
    std::optional<domain::DecodedDocument> document;
    QString systemError;
    domain::RleErrorInfo decodeError;
};

/// File I/O only; RLE parsing is delegated to the pure domain codec.
class RleFileService
{
public:
    RleReadResult read(const QString& path) const;
    bool write(const QString& path, const QString& text, QString* error) const;
};

} // namespace lifegame::service
