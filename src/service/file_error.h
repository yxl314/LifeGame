#pragma once

namespace lifegame::service
{

/// Recoverable infrastructure failures. The UI turns these into one message.
enum class FileError
{
    None = 0,
    OpenFailed,
    ReadFailed,
    WriteFailed,
    DecodeFailed,
    EncodeFailed,
    NullImage,
    TooLarge
};

constexpr const char* fileErrorName(FileError error) noexcept
{
    switch (error)
    {
    case FileError::None:
        return "None";
    case FileError::OpenFailed:
        return "OpenFailed";
    case FileError::ReadFailed:
        return "ReadFailed";
    case FileError::WriteFailed:
        return "WriteFailed";
    case FileError::DecodeFailed:
        return "DecodeFailed";
    case FileError::EncodeFailed:
        return "EncodeFailed";
    case FileError::NullImage:
        return "NullImage";
    case FileError::TooLarge:
        return "TooLarge";
    }
    return "Unknown";
}

} // namespace lifegame::service
