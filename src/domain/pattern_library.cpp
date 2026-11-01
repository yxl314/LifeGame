#include "domain/pattern_library.h"

#include <algorithm>
#include <vector>

namespace lifegame::domain
{

const std::vector<Pattern>& builtinPatterns()
{
    static const std::vector<Pattern> patterns = {
        Pattern::fromAscii("glider", {".O.", "..O", "OOO"}),

        Pattern::fromAscii("gosper_gun",
                           {
                               "........................O...........",
                               "......................O.O...........",
                               "............OO......OO............OO",
                               "...........O...O....OO............OO",
                               "OO........O.....O...OO..............",
                               "OO........O...O.OO....O.O...........",
                               "..........O.....O.......O...........",
                               "...........O...O....................",
                               "............OO......................",
                           }),

        Pattern::fromAscii("pulsar",
                           {
                               "..OOO...OOO..",
                               ".............",
                               "O....O.O....O",
                               "O....O.O....O",
                               "O....O.O....O",
                               "..OOO...OOO..",
                               ".............",
                               "..OOO...OOO..",
                               "O....O.O....O",
                               "O....O.O....O",
                               "O....O.O....O",
                               ".............",
                               "..OOO...OOO..",
                           }),

        Pattern::fromAscii("blinker", {"OOO"}),
        Pattern::fromAscii("block", {"OO", "OO"}),
        Pattern::fromAscii("beehive", {".OO.", "O..O", ".OO."}),
        Pattern::fromAscii("toad", {".OOO", "OOO."}),
        Pattern::fromAscii("beacon", {"OO..", "OO..", "..OO", "..OO"}),
        Pattern::fromAscii("pentadecathlon", {"..O....O..", "OO.OOOO.OO", "..O....O.."}),
        Pattern::fromAscii("lwss", {".O..O", "O....", "O...O", "OOOO."}),
        Pattern::fromAscii("acorn", {".O.....", "...O...", "OO..OOO"}),
        Pattern::fromAscii("r_pentomino", {".OO", "OO.", ".O."}),
        Pattern::fromAscii("diehard", {"......O.", "OO......", ".O...OOO"}),
    };
    return patterns;
}

const Pattern* findBuiltinPattern(std::string_view id)
{
    const auto& patterns = builtinPatterns();
    const auto it = std::find_if(patterns.begin(), patterns.end(), [id](const Pattern& pattern) {
        return pattern.id() == id;
    });
    return it == patterns.end() ? nullptr : &*it;
}

} // namespace lifegame::domain
