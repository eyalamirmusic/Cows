#include "Level.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
Vec2 pushedOutOf(const Collider& collider, Vec2 point, float radius, float feet)
{
    if (collider.top <= feet + stepUp)
        return point;

    auto offset = point - collider.center;
    auto reach = collider.radius + radius;
    auto gap = length(offset);

    if (gap >= reach)
        return point;

    auto away = gap > 1e-4f ? offset / gap : Vec2 {1.f, 0.f};
    return collider.center + away * reach;
}

bool standsOn(const Collider& collider, Vec2 point, float feet)
{
    return collider.top <= feet + stepUp
           && distance(point, collider.center) < collider.radius;
}
} // namespace

bool Gap::contains(Vec2 point) const
{
    return std::abs(point.x - center.x) < half.x
           && std::abs(point.y - center.y) < half.y;
}

Vec2 Level::pushedOut(Vec2 point, float radius, float feet) const
{
    for (auto pass = 0; pass < 2; ++pass)
    {
        for (const auto& collider: colliders)
            point = pushedOutOf(collider, point, radius, feet);

        for (const auto& mover: movers)
            point = pushedOutOf(mover.collider, point, radius, feet);

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

    return keptInGap(point, radius, feet);
}

Vec2 Level::keptInGap(Vec2 point, float radius, float feet) const
{
    if (gaps.empty() || feet + stepUp >= 0.f || overGap(point))
        return point;

    auto nearest = gaps.begin();
    auto closest = 1e30f;

    for (auto gap = gaps.begin(); gap != gaps.end(); ++gap)
    {
        auto offset = absolute(point - gap->center) - gap->half;
        auto away = length(Vec2 {std::max(offset.x, 0.f), std::max(offset.y, 0.f)});

        if (away < closest)
        {
            closest = away;
            nearest = gap;
        }
    }

    auto room = Vec2 {std::max(nearest->half.x - radius, 0.f),
                      std::max(nearest->half.y - radius, 0.f)};
    return {
        std::clamp(point.x, nearest->center.x - room.x, nearest->center.x + room.x),
        std::clamp(point.y, nearest->center.y - room.y, nearest->center.y + room.y)};
}

float Level::floorAt(Vec2 point, float feet) const
{
    auto floor = groundAt(point);

    for (const auto& collider: colliders)
        if (standsOn(collider, point, feet))
            floor = std::max(floor, collider.top);

    for (const auto& mover: movers)
        if (standsOn(mover.collider, point, feet))
            floor = std::max(floor, mover.collider.top);

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

float Level::groundAt(Vec2 point) const
{
    auto ground = 0.f;

    for (const auto& gap: gaps)
        if (gap.contains(point))
            ground = std::min(ground, gap.depth);

    return ground;
}

bool Level::overGap(Vec2 point) const
{
    return std::any_of(gaps.begin(),
                       gaps.end(),
                       [&](const Gap& gap) { return gap.contains(point); });
}

bool Level::hasGround(Vec2 point) const
{
    if (!overGap(point))
        return true;

    auto bridged = [&](const Block& block) { return block.contains(point); };
    auto covered = [&](const Collider& collider)
    { return distance(point, collider.center) < collider.radius; };

    return std::any_of(blocks.begin(), blocks.end(), bridged)
           || std::any_of(colliders.begin(), colliders.end(), covered);
}

bool Level::onMover(Vec2 point, float feet) const
{
    return std::any_of(movers.begin(),
                       movers.end(),
                       [&](const Mover& mover)
                       {
                           return standsOn(mover.collider, point, feet)
                                  && std::abs(mover.collider.top - feet) < 0.01f;
                       });
}

void Level::update(float seconds)
{
    moving.clear();

    for (auto& mover: movers)
    {
        mover.collider.center = mover.positionAt(seconds);
        moving.add(Shape::Barrel,
                   makeInstance(mover.modelAt(seconds), mover.material));
    }
}
} // namespace Cows
