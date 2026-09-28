#pragma once

#include "Levels/Segments/Segment.h"

namespace Cows
{
constexpr auto jumpLineLength = 9.f;

// A line across the whole arena, through its middle, too tall to step over
// and low enough to jump: felled logs or a post-and-rail fence.
struct JumpLineSegment final : Segment
{
    enum class Obstacle
    {
        Log,
        Fence
    };

    explicit JumpLineSegment(Obstacle obstacleToUse,
                             float lengthToUse = jumpLineLength);

    void build(Level& level, Layout& layout, Region region) const override;

    Obstacle obstacle = Obstacle::Log;
};
} // namespace Cows
