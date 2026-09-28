#include "Levels/Segments/JumpLineSegment.h"
#include "Props/Fence.h"
#include "Props/Log.h"

#include <algorithm>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto pieceOverlap = 0.4f;
constexpr auto lineJitter = 0.3f;
} // namespace

JumpLineSegment::JumpLineSegment(Obstacle obstacleToUse, float lengthToUse)
    : obstacle(obstacleToUse)
{
    length = lengthToUse;
}

void JumpLineSegment::build(Level& level, Layout& layout, Region region) const
{
    auto z = region.center().y;
    auto x = region.min.x;

    while (region.max.x - x > pieceOverlap * 2.f)
    {
        auto piece = std::min(layout.between(5.f, 8.f), region.max.x - x);
        auto at =
            Vec2 {x + piece * 0.5f, z + layout.between(-lineJitter, lineJitter)};

        if (obstacle == Obstacle::Log)
            addLog(level, at, piece);
        else
            addFence(level, at, piece);

        x += piece - pieceOverlap;
    }
}
} // namespace Cows
