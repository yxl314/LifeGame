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

/// Output port used by FileController and replaceable in tests.
class RleFilePort
{
public:
    virtual ~RleFilePort() = default;
    virtual RleReadResult read(const QString& path) const = 0;
    virtual FileError write(const QString& path, const QString& text, QString* detail = nullptr) const = 0;
};

/// File I/O only; RLE parsing is delegated to the pure domain codec.
class RleFileService : public RleFilePort
{
public:
    RleReadResult read(const QString& path) const override;

    FileError write(const QString& path, const QString& text, QString* detail = nullptr) const override;
};

} // namespace lifegame::service
