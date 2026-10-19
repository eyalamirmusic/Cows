#pragma once

#include "Levels/Segments/Segment.h"

namespace Cows
{
constexpr auto jumpLineLength = 9.f;

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
