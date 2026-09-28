#pragma once

#include "Levels/LevelTemplate.h"

#include <cstdint>

namespace Cows
{
constexpr auto pathHalfWidth = 3.f;

// The segments' regions, one after another along z, the first at the south
// (+z) edge; together they are centred on the origin.
Vector<Region> segmentRegions(const LevelTemplate& levelTemplate);

// Lays out a level from a template and a seed; the same seed always gives the
// same level. The player starts in the middle of the first segment, on the
// critical path: a strip from there to the last segment that the populate
// pass keeps clear. Each segment is built, the biome regions populated, and
// the goal (where she hides) chosen in the last segment.
Level generate(const LevelTemplate& levelTemplate, std::uint32_t seed);
} // namespace Cows
