#include "Level.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
Vec2 Level::pushedOut(Vec2 point, float radius, float feet) const
{
    for (auto pass = 0; pass < 2; ++pass)
    {
        for (const auto& collider: colliders)
        {
            if (collider.top <= feet + stepUp)
                continue;

            auto offset = point - collider.center;
            auto reach = collider.radius + radius;
            auto gap = length(offset);

            if (gap >= reach)
                continue;

            auto away = gap > 1e-4f ? offset / gap : Vec2 {1.f, 0.f};
            point = collider.center + away * reach;
        }

        for (const auto& block: blocks)
        {
            auto closest = block.closestTo(point);

            if (block.heightAt(closest) <= feet + stepUp)
                continue;

            auto offset = point - closest;
            auto gap = length(offset);

            if (gap >= radius)
                continue;

            if (gap > 1e-4f)
            {
                point = closest + offset / gap * radius;
                continue;
            }

            auto inside = point - block.center;
            auto depth = block.half - absolute(inside);

            if (depth.x < depth.y)
                point.x =
                    block.center.x + std::copysign(block.half.x + radius, inside.x);
            else
                point.y =
                    block.center.y + std::copysign(block.half.y + radius, inside.y);
        }
    }

    return point;
}

float Level::floorAt(Vec2 point, float feet) const
{
    auto floor = 0.f;

    for (const auto& collider: colliders)
        if (collider.top <= feet + stepUp
            && distance(point, collider.center) < collider.radius)
            floor = std::max(floor, collider.top);

    for (const auto& block: blocks)
    {
        if (!block.contains(point))
            continue;

        auto height = block.heightAt(point);

        if (height <= feet + stepUp)
            floor = std::max(floor, height);
    }

    return floor;
}

bool Level::isFree(Vec2 point, float margin) const
{
    return std::none_of(blocks.begin(),
                        blocks.end(),
                        [&](const Block& block)
                        { return block.contains(point, margin); });
}
} // namespace Cows
