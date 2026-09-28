#pragma once

#include "Levels/Segments/Segment.h"

namespace Cows
{
// A level's content: its segments in order from the start (south, +z) to the
// goal (north, -z), how wide it is, how far off the middle the critical path
// may run, and which way the player faces at the start.
struct LevelTemplate final
{
    SegmentList segments;
    float width = arenaSize;
    float pathReach = 0.f;
    float startHeading = 0.f;
};
} // namespace Cows
