#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace lifegame::domain
{

struct RuleParseResult;
class Rule;

enum class RuleParseError
{
    None = 0,
    Empty,
    MissingSeparator,
    InvalidCharacter
};

constexpr const char* ruleParseErrorName(RuleParseError error) noexcept
{
    switch (error)
    {
    case RuleParseError::None:
        return "None";
    case RuleParseError::Empty:
        return "Empty";
    case RuleParseError::MissingSeparator:
        return "MissingSeparator";
    case RuleParseError::InvalidCharacter:
        return "InvalidCharacter";
    }
    return "Unknown";
}

/// A Life-like rule in B/S notation.
///
/// The rule stores two 9-bit masks. Bit n means "neighbour count n".
/// The domain type deliberately carries no display name: localized names live
/// in the view layer.
class Rule
{
public:
    /// Conway's rule, B3/S23.
    constexpr Rule() = default;

    constexpr Rule(std::uint16_t birth, std::uint16_t survival) noexcept
        : m_birth(static_cast<std::uint16_t>(birth & 0x01FFu)),
          m_survival(static_cast<std::uint16_t>(survival & 0x01FFu))
    {
    }

    constexpr bool births(int neighbours) const noexcept
    {
        return neighbours >= 0 && neighbours <= 8 && (m_birth & (1u << neighbours)) != 0;
    }

    constexpr bool survives(int neighbours) const noexcept
    {
        return neighbours >= 0 && neighbours <= 8 && (m_survival & (1u << neighbours)) != 0;
    }

    constexpr std::uint16_t birthMask() const noexcept
    {
        return m_birth;
    }

    constexpr std::uint16_t survivalMask() const noexcept
    {
        return m_survival;
    }

    /// Canonical notation, always "B<digit>/S<digit>".
    std::string notation() const;

    /**
     * Parse B/S notation into `rule`.
     *
     * Accepts both "B3/S23" and the legacy "3/23" form, is case-insensitive
     * and ignores surrounding whitespace. Returns a typed error code; an empty
     * birth or survival set (e.g. "B2/S") is valid.
     */
    static RuleParseResult parse(std::string_view text);

    friend constexpr bool operator==(const Rule& lhs, const Rule& rhs) noexcept
    {
        return lhs.m_birth == rhs.m_birth && lhs.m_survival == rhs.m_survival;
    }

    friend constexpr bool operator!=(const Rule& lhs, const Rule& rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    std::uint16_t m_birth = static_cast<std::uint16_t>(1u << 3);
    std::uint16_t m_survival = static_cast<std::uint16_t>((1u << 2) | (1u << 3));
};

struct RuleParseResult
{
    RuleParseError error = RuleParseError::None;
    Rule rule;
};

/// A built-in rule with a stable, non-localized identifier.
struct RulePreset
{
    std::string_view id;
    Rule rule;
};

/// Built-in rules in menu order: conway, highlife, daynight, seeds, maze.
const std::vector<RulePreset>& builtinRulePresets();

const RulePreset* findRulePreset(std::string_view id);
const RulePreset* findRulePresetByNotation(std::string_view notation);

} // namespace lifegame::domain
