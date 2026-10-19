#include "Level.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

using namespace Maths;

namespace Cows
{
namespace
{
Vec2 pushedOutOf(const Collider& collider, Vec2 point, float radius, float feet)
{
    if (collider.top <= feet + maxStepUpHeight)
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
    return collider.top <= feet + maxStepUpHeight
           && distance(point, collider.center) < collider.radius;
}

bool floats(const Block& block)
{
    return block.bottom > 0.f;
}

bool passesUnder(const Block& block, Vec2 at, float feet)
{
    return floats(block) && feet < block.bottom
           && feet + maxStepUpHeight < block.heightAt(at);
}

struct HalfSpace final
{
    Vec3 normal;
    float offset = 0.f;
};

// A floating ramp is a slab a step deep: as far down as passesUnder counts as on it.
std::array<HalfSpace, 6> sidesOf(const Block& block)
{
    auto low = block.center - block.half;
    auto high = block.center + block.half;
    auto sides =
        std::array<HalfSpace, 6> {HalfSpace {{1.f, 0.f, 0.f}, high.x},
                                  HalfSpace {{-1.f, 0.f, 0.f}, -low.x},
                                  HalfSpace {{0.f, 0.f, 1.f}, high.y},
                                  HalfSpace {{0.f, 0.f, -1.f}, -low.y},
                                  HalfSpace {{0.f, -1.f, 0.f}, -block.bottom},
                                  HalfSpace {{0.f, 1.f, 0.f}, block.top}};

    if (lengthSquared(block.rise) > 0.f)
    {
        auto along = dot(absolute(block.rise), block.half);
        auto slope = (block.top - block.bottom) * 0.5f / along;
        sides[5] = {{-slope * block.rise.x, 1.f, -slope * block.rise.y},
                    (block.top + block.bottom) * 0.5f
                        - slope * dot(block.center, block.rise)};

        if (floats(block))
            sides[4] = {-sides[5].normal, maxStepUpHeight - sides[5].offset};
    }

    return sides;
}

// Cyrus-Beck clipping; nothing when the segment misses or starts inside.
std::optional<float> entryInto(const Block& block, Vec3 from, Vec3 to)
{
    auto direction = to - from;
    auto entry = 0.f;
    auto exit = 1.f;
    auto inside = true;

    for (const auto& side: sidesOf(block))
    {
        auto room = side.offset - dot(side.normal, from);
        auto closing = dot(side.normal, direction);
        inside = inside && room >= 0.f;

        if (closing == 0.f)
        {
            if (room < 0.f)
                return std::nullopt;

            continue;
        }

        auto crossing = room / closing;

        if (closing < 0.f)
            entry = std::max(entry, crossing);
        else
            exit = std::min(exit, crossing);
    }

    if (inside || entry > exit)
        return std::nullopt;

    return entry;
}
} // namespace

float headingToward(Vec2 direction)
{
    return std::atan2(-direction.y, direction.x);
}

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

            if (passesUnder(block, closest, feet))
                continue;

            if (block.heightAt(closest) <= feet + maxStepUpHeight)
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
    if (gaps.empty() || feet + maxStepUpHeight >= 0.f || overGap(point))
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
        if (!block.contains(point) || passesUnder(block, point, feet))
            continue;

        auto height = block.heightAt(point);

        if (height <= feet + maxStepUpHeight)
            floor = std::max(floor, height);
    }

    return floor;
}

float Level::ceilingAt(Vec2 point, float feet) const
{
    auto ceiling = std::numeric_limits<float>::infinity();

    for (const auto& block: blocks)
        if (block.contains(point) && passesUnder(block, point, feet))
            ceiling = std::min(ceiling, block.bottom);

    return ceiling;
}

bool Level::isFree(Vec2 point, float margin) const
{
    return std::none_of(blocks.begin(),
                        blocks.end(),
                        [&](const Block& block)
                        { return block.contains(point, margin); });
}

Vec3 Level::clearReach(Vec3 from, Vec3 to, float margin) const
{
    auto clear = 1.f;

    for (const auto& block: blocks)
        if (auto entry = entryInto(block, from, to))
            clear = std::min(clear, *entry);

    if (clear >= 1.f)
        return to;

    auto span = length(to - from);
    auto reach = std::max(clear * span - margin, 0.f);
    return from + (to - from) * (reach / span);
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
        auto model = mover.modelAt(seconds);

        for (const auto& part: mover.parts)
            moving.add(part.shape, makeInstance(model, part.material));
    }
}
} // namespace Cows
