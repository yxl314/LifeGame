#pragma once

/**
 * @file rle_file_service.h
 * @brief RLE file I/O port and service.
 */

#include <QString>

#include <optional>

#include "domain/rle.h"
#include "service/file_error.h"

namespace lifegame::service
{

/// @brief Result of reading and decoding an RLE file.
struct RleReadResult
{
    std::optional<domain::DecodedDocument> document; ///< Decoded document on success.
    FileError error = FileError::None;               ///< Typed file error.
    domain::RleErrorInfo decodeError;                ///< RLE parser details on decode failure.
    QString detail;                                  ///< Optional system detail.
};

/// Output port used by FileController and replaceable in tests.
/// @brief Replaceable RLE file I/O port.
class RleFilePort
{
public:
    virtual ~RleFilePort() = default;

    /// @param path File path.
    /// @return Decoded document or a typed file/decode error.
    virtual RleReadResult read(const QString& path) const = 0;

    /**
     * @brief Atomically write RLE text.
     * @param path Destination path.
     * @param text RLE text.
     * @param detail Optional system error detail.
     * @return FileError code.
     */
    virtual FileError write(const QString& path, const QString& text, QString* detail = nullptr) const = 0;
};

/// @brief File I/O only; RLE parsing is delegated to the pure domain codec.
class RleFileService : public RleFilePort
{
public:
    RleReadResult read(const QString& path) const override;
    FileError write(const QString& path, const QString& text, QString* detail = nullptr) const override;
};

} // namespace lifegame::service
