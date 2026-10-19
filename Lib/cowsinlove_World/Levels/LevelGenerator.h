#pragma once

#include "Levels/LevelTemplate.h"

#include <cstdint>

namespace Cows
{
constexpr auto pathHalfWidth = 3.f;

// From the south (+z) edge northwards, centred on the origin.
Vector<Region> segmentRegions(const LevelTemplate& levelTemplate);

Level generate(const LevelTemplate& levelTemplate, std::uint32_t seed);
} // namespace Cows
