#include "domain/rle.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace lifegame::domain
{

namespace
{

constexpr int kMaxExtent = kMaxBoardExtent;

std::string_view trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0)
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0)
    {
        text.remove_suffix(1);
    }
    return text;
}

bool startsWithHeaderKey(std::string_view line)
{
    return !line.empty() && (line.front() == 'x' || line.front() == 'X') &&
           line.find('=') != std::string_view::npos;
}

bool parseNonNegativeInt(std::string_view text, int* value)
{
    if (text.empty())
    {
        return false;
    }

    std::int64_t result = 0;
    for (const char ch : text)
    {
        if (ch < '0' || ch > '9')
        {
            return false;
        }
        result = result * 10 + (ch - '0');
        if (result > 1'000'000)
        {
            return false;
        }
    }
    *value = static_cast<int>(result);
    return true;
}

struct Header
{
    int width = 0;
    int height = 0;
    bool hasWidth = false;
    bool hasHeight = false;
    std::string rule;
};

RleErrorInfo parseHeader(std::string_view line, Header* header)
{
    std::size_t start = 0;
    while (start <= line.size())
    {
        const std::size_t comma = line.find(',', start);
        const std::size_t end = comma == std::string_view::npos ? line.size() : comma;
        const std::string_view token = trim(line.substr(start, end - start));

        if (!token.empty())
        {
            const std::size_t equals = token.find('=');
            if (equals == std::string_view::npos)
            {
                return {RleError::InvalidHeader, start};
            }

            const std::string_view key = trim(token.substr(0, equals));
            const std::string_view value = trim(token.substr(equals + 1));

            if (key == "x" || key == "X")
            {
                if (!parseNonNegativeInt(value, &header->width))
                {
                    return {RleError::InvalidHeader, start};
                }
                header->hasWidth = true;
            }
            else if (key == "y" || key == "Y")
            {
                if (!parseNonNegativeInt(value, &header->height))
                {
                    return {RleError::InvalidHeader, start};
                }
                header->hasHeight = true;
            }
            else if (key == "rule" || key == "Rule")
            {
                if (!value.empty())
                {
                    const std::optional<Rule> rule = Rule::fromNotation(value);
                    if (!rule.has_value())
                    {
                        return {RleError::InvalidRule, start};
                    }
                    header->rule = rule->notation();
                }
            }
        }

        if (comma == std::string_view::npos)
        {
            break;
        }
        start = comma + 1;
    }

    return {};
}

void appendRun(std::string* output, int count, char tag)
{
    if (count <= 0)
    {
        return;
    }
    if (count > 1)
    {
        output->append(std::to_string(count));
    }
    output->push_back(tag);
}

void wrapLine(std::string* output, std::size_t* lineLength)
{
    if (*lineLength >= 70)
    {
        output->push_back('\n');
        *lineLength = 0;
    }
}

std::string sanitizeTitle(std::string_view title)
{
    std::string result(title);
    for (char& ch : result)
    {
        if (ch == '\n' || ch == '\r')
        {
            ch = ' ';
        }
    }
    return result;
}

} // namespace

RleDecodeResult decodeRle(std::string_view text)
{
    Header header;
    std::string title;
    std::string description;
    std::string body;
    bool sawHeader = false;

    std::size_t lineStart = 0;
    while (lineStart <= text.size())
    {
        const std::size_t lineEnd = text.find('\n', lineStart);
        const std::size_t end = lineEnd == std::string_view::npos ? text.size() : lineEnd;
        std::string_view line = text.substr(lineStart, end - lineStart);
        if (!line.empty() && line.back() == '\r')
        {
            line.remove_suffix(1);
        }

        const std::string_view clean = trim(line);
        if (!clean.empty())
        {
            if (clean.front() == '#')
            {
                if (clean.size() >= 3 && (clean[1] == 'N' || clean[1] == 'n') && clean[2] == ' ')
                {
                    title = std::string(trim(clean.substr(3)));
                }
                else if (clean.size() >= 3 && (clean[1] == 'C' || clean[1] == 'c') && clean[2] == ' ')
                {
                    if (!description.empty())
                    {
                        description.push_back(' ');
                    }
                    description += std::string(trim(clean.substr(3)));
                }
            }
            else if (!sawHeader && startsWithHeaderKey(clean))
            {
                sawHeader = true;
                const RleErrorInfo error = parseHeader(clean, &header);
                if (error.code != RleError::None)
                {
                    return {std::nullopt, error};
                }
            }
            else
            {
                body.append(clean);
                body.push_back('\n');
            }
        }

        if (lineEnd == std::string_view::npos)
        {
            break;
        }
        lineStart = lineEnd + 1;
    }

    if (body.empty())
    {
        return {std::nullopt, {RleError::EmptyBody, 0}};
    }

    if (header.hasWidth && header.width > kMaxExtent)
    {
        return {std::nullopt, {RleError::TooLarge, 0}};
    }
    if (header.hasHeight && header.height > kMaxExtent)
    {
        return {std::nullopt, {RleError::TooLarge, 0}};
    }

    std::vector<std::vector<Cell>> rows(1);
    std::int64_t row = 0;
    std::int64_t col = 0;
    std::int64_t run = 0;
    std::int64_t maxCol = 0;
    bool finished = false;

    for (std::size_t i = 0; i < body.size() && !finished; ++i)
    {
        const char ch = body[i];
        if (ch >= '0' && ch <= '9')
        {
            run = run * 10 + (ch - '0');
            if (run > static_cast<std::int64_t>(kMaxExtent) * kMaxExtent)
            {
                return {std::nullopt, {RleError::RunTooLong, i}};
            }
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) != 0)
        {
            continue;
        }

        const std::int64_t count = run > 0 ? run : 1;
        run = 0;

        switch (ch)
        {
        case 'b':
        case 'B':
            if (col + count > kMaxExtent)
            {
                return {std::nullopt, {RleError::TooLarge, i}};
            }
            col += count;
            maxCol = std::max(maxCol, col);
            break;

        case 'o':
        case 'O':
        {
            if (row >= kMaxExtent || col + count > kMaxExtent)
            {
                return {std::nullopt, {RleError::TooLarge, i}};
            }
            if (static_cast<std::int64_t>(rows.size()) <= row)
            {
                rows.resize(static_cast<std::size_t>(row) + 1);
            }
            std::vector<Cell>& target = rows[static_cast<std::size_t>(row)];
            if (static_cast<std::int64_t>(target.size()) < col + count)
            {
                target.resize(static_cast<std::size_t>(col + count), kDead);
            }
            for (std::int64_t offset = 0; offset < count; ++offset)
            {
                target[static_cast<std::size_t>(col + offset)] = kAlive;
            }
            col += count;
            maxCol = std::max(maxCol, col);
            break;
        }

        case '$':
            row += count;
            col = 0;
            if (row >= kMaxExtent)
            {
                return {std::nullopt, {RleError::TooLarge, i}};
            }
            if (static_cast<std::int64_t>(rows.size()) <= row)
            {
                rows.resize(static_cast<std::size_t>(row) + 1);
            }
            break;

        case '!':
            finished = true;
            break;

        default:
            return {std::nullopt, {RleError::InvalidCharacter, i}};
        }
    }

    const int contentWidth = std::max(1, static_cast<int>(maxCol));
    const int contentHeight = std::max(1, static_cast<int>(rows.size()));
    const int width = std::max(header.hasWidth ? header.width : 0, contentWidth);
    const int height = std::max(header.hasHeight ? header.height : 0, contentHeight);

    if (width > kMaxExtent || height > kMaxExtent)
    {
        return {std::nullopt, {RleError::TooLarge, 0}};
    }

    DecodedDocument document;
    document.title = title;
    document.description = description;
    document.rule = header.rule;

    Pattern& pattern = document.pattern;
    pattern.id = "rle";
    pattern.width = width;
    pattern.height = height;
    pattern.cells.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), kDead);

    const int copyRows = std::min(height, static_cast<int>(rows.size()));
    for (int r = 0; r < copyRows; ++r)
    {
        const int copyCols = std::min(width, static_cast<int>(rows[static_cast<std::size_t>(r)].size()));
        for (int c = 0; c < copyCols; ++c)
        {
            pattern.cells[static_cast<std::size_t>(r) * static_cast<std::size_t>(width) +
                          static_cast<std::size_t>(c)] = rows[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)];
        }
    }

    return {std::move(document), {}};
}

std::string encodeRle(const Grid& board, const Rule& rule, std::string_view title)
{
    const int width = board.width();
    const int height = board.height();
    if (width <= 0 || height <= 0)
    {
        return "!";
    }

    int lastLiveRow = -1;
    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            if (board.cell(row, col))
            {
                lastLiveRow = row;
                break;
            }
        }
    }

    std::string output;
    if (!title.empty())
    {
        output += "#N " + sanitizeTitle(title) + "\n";
    }
    output += "x = " + std::to_string(width) + ", y = " + std::to_string(height) +
              ", rule = " + rule.notation() + "\n";

    std::size_t lineLength = 0;
    for (int row = 0; row <= lastLiveRow; ++row)
    {
        int lastInRow = -1;
        for (int col = 0; col < width; ++col)
        {
            if (board.cell(row, col))
            {
                lastInRow = col;
            }
        }

        int col = 0;
        while (col <= lastInRow)
        {
            const bool alive = board.cell(row, col);
            int run = 1;
            while (col + run <= lastInRow && board.cell(row, col + run) == alive)
            {
                ++run;
            }

            wrapLine(&output, &lineLength);
            appendRun(&output, run, alive ? 'o' : 'b');
            lineLength += static_cast<std::size_t>(run > 1 ? std::to_string(run).size() + 1 : 1);
            col += run;
        }

        if (row < lastLiveRow)
        {
            output.push_back('$');
            ++lineLength;
        }
    }

    output.push_back('!');
    output.push_back('\n');
    return output;
}

} // namespace lifegame::domain
