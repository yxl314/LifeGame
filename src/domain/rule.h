#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lifegame::domain
{

/// A Life-like rule in B/S notation.
///
/// The rule stores two 9-bit masks. Bit n means "neighbour count n".
/// The domain type deliberately carries no display name: localized names live
/// in the view layer.
class Rule
{
public:
    /// Conway's rule, B3/S23.
    Rule() = default;

    Rule(std::uint16_t birth, std::uint16_t survival);

    bool births(int neighbours) const;
    bool survives(int neighbours) const;

    std::uint16_t birthMask() const
    {
        return m_birth;
    }

    std::uint16_t survivalMask() const
    {
        return m_survival;
    }

    /// Canonical notation, always "B<digit>/S<digit>".
    std::string notation() const;

    /**
     * Parse B/S notation.
     *
     * Accepts both "B3/S23" and the legacy "3/23" form, is case-insensitive
     * and ignores surrounding whitespace. Returns nullopt for malformed input;
     * an empty birth or survival set (e.g. "B2/S") is valid.
     */
    static std::optional<Rule> fromNotation(std::string_view text);

    friend bool operator==(const Rule& lhs, const Rule& rhs)
    {
        return lhs.m_birth == rhs.m_birth && lhs.m_survival == rhs.m_survival;
    }

    friend bool operator!=(const Rule& lhs, const Rule& rhs)
    {
        return !(lhs == rhs);
    }

private:
    std::uint16_t m_birth = static_cast<std::uint16_t>(1u << 3);
    std::uint16_t m_survival = static_cast<std::uint16_t>((1u << 2) | (1u << 3));
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
