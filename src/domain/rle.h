#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "domain/grid.h"
#include "domain/pattern.h"
#include "domain/rule.h"

namespace lifegame::domain
{

/// Stable error codes so the UI can localize messages; the domain never
/// returns user-facing prose.
enum class RleError
{
    None = 0,
    EmptyBody,
    InvalidHeader,
    InvalidCharacter,
    RunTooLong,
    TooLarge,
    InvalidRule,
    MissingTerminator
};

constexpr const char* rleErrorName(RleError error) noexcept
{
    switch (error)
    {
    case RleError::None:
        return "None";
    case RleError::EmptyBody:
        return "EmptyBody";
    case RleError::InvalidHeader:
        return "InvalidHeader";
    case RleError::InvalidCharacter:
        return "InvalidCharacter";
    case RleError::RunTooLong:
        return "RunTooLong";
    case RleError::TooLarge:
        return "TooLarge";
    case RleError::InvalidRule:
        return "InvalidRule";
    case RleError::MissingTerminator:
        return "MissingTerminator";
    }
    return "Unknown";
}

struct RleErrorInfo
{
    RleError code = RleError::None;

    /// Original byte offset in the input (0-based).
    std::size_t position = 0;

    /// 1-based source location for user-facing diagnostics.
    int line = 1;
    int column = 1;
};

struct DecodedDocument
{
    Pattern pattern;
    std::string title;
    std::string description;
    std::string rule; // canonical notation, empty when absent
};

struct RleDecodeResult
{
    std::optional<DecodedDocument> document;
    RleErrorInfo error;
};

/**
 * Decode RLE text.
 *
 * The parser is intentionally strict and bounded: a malformed or hostile file
 * can never make the column/row counters overflow or make the decoder write
 * outside its buffers.
 */
RleDecodeResult decodeRle(std::string_view text);

/// Encode the full board size in the header and stop the body at the last live row.
std::string encodeRle(const Grid& board, const Rule& rule, std::string_view title = {});

} // namespace lifegame::domain
