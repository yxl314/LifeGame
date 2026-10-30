#pragma once

#include <string_view>
#include <vector>

#include "domain/pattern.h"

namespace lifegame::domain
{

/**
 * Built-in patterns (stable IDs + shapes only).
 *
 * Display names and descriptions live in the view layer.
 */
const std::vector<Pattern>& builtinPatterns();

const Pattern* findBuiltinPattern(std::string_view id);

} // namespace lifegame::domain
