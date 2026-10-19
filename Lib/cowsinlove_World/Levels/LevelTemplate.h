#pragma once

#include "Levels/Segments/Segment.h"

namespace Cows
{
// Segments run from the start (south, +z) to the goal (north, -z).
struct LevelTemplate final
{
    SegmentList segments;
    float width = arenaSize;
    float pathReach = 0.f;
    float startHeading = 0.f;
};
} // namespace Cows
