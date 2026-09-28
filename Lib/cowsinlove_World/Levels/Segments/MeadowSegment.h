#pragma once

#include "Levels/Segments/Segment.h"

namespace Cows
{
// Open meadow: nothing built, dressed by the meadow biome.
struct MeadowSegment final : Segment
{
    explicit MeadowSegment(float lengthToUse);
};
} // namespace Cows
