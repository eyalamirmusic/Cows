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
constexpr auto smallestPushDistance = 1e-4f;
constexpr auto onMoverTolerance = 0.01f;

struct Stance final
{
    Vec2 at;
    float feet = 0.f;
};

struct Body final
{
    Stance stance;
    float radius = 0.f;
};

Vec2 getDirectionAwayFrom(Vec2 offset)
{
    auto gap = length(offset);

    if (gap > smallestPushDistance)
        return offset / gap;

    return {1.f, 0.f};
}

Vec2 pushedOutOf(const Collider& collider, const Body& body)
{
    auto point = body.stance.at;

    if (collider.top <= body.stance.feet + maxStepUpHeight)
        return point;

    auto offset = point - collider.center;
    auto reach = collider.radius + body.radius;
    auto gap = length(offset);

    if (gap >= reach)
        return point;

    return collider.center + getDirectionAwayFrom(offset) * reach;
}

bool standsOn(const Collider& collider, const Stance& stance)
{
    return collider.top <= stance.feet + maxStepUpHeight
           && distance(stance.at, collider.center) < collider.radius;
}

bool floats(const Block& block)
{
    return block.bottom > 0.f;
}

bool passesUnder(const Block& block, const Stance& stance)
{
    if (!floats(block))
        return false;

    return stance.feet < block.bottom
           && stance.feet + maxStepUpHeight < block.heightAt(stance.at);
}

Vec2 pushedOutOfBlockInterior(const Block& block, const Body& body)
{
    auto point = body.stance.at;
    auto inside = point - block.center;
    auto depth = block.half - absolute(inside);

    if (depth.x < depth.y)
        point.x =
            block.center.x + std::copysign(block.half.x + body.radius, inside.x);
    else
        point.y =
            block.center.y + std::copysign(block.half.y + body.radius, inside.y);

    return point;
}

bool isBlockInTheWay(const Block& block, const Stance& closest)
{
    if (passesUnder(block, closest))
        return false;

    auto canStepOnto = block.heightAt(closest.at) <= closest.feet + maxStepUpHeight;
    return !canStepOnto;
}

Vec2 pushedOutOf(const Block& block, const Body& body)
{
    auto point = body.stance.at;
    auto closest = block.closestTo(point);

    if (!isBlockInTheWay(block, {closest, body.stance.feet}))
        return point;

    auto offset = point - closest;
    auto gap = length(offset);

    if (gap >= body.radius)
        return point;

    if (gap > smallestPushDistance)
        return closest + offset / gap * body.radius;

    return pushedOutOfBlockInterior(block, body);
}

float getFloorOfCollider(const Collider& collider, const Stance& stance)
{
    if (standsOn(collider, stance))
        return collider.top;

    return -std::numeric_limits<float>::infinity();
}

float getFloorOfBlock(const Block& block, const Stance& stance)
{
    if (!block.contains(stance.at) || passesUnder(block, stance))
        return -std::numeric_limits<float>::infinity();

    auto height = block.heightAt(stance.at);

    if (height <= stance.feet + maxStepUpHeight)
        return height;

    return -std::numeric_limits<float>::infinity();
}

float getDistanceOutsideGap(const Gap& gap, Vec2 point)
{
    auto offset = absolute(point - gap.center) - gap.half;
    return length(Vec2 {std::max(offset.x, 0.f), std::max(offset.y, 0.f)});
}

const Gap& findNearestGap(const Vector<Gap>& gaps, Vec2 point)
{
    auto nearest = gaps.begin();
    auto closest = 1e30f;

    for (auto gap = gaps.begin(); gap != gaps.end(); ++gap)
    {
        auto away = getDistanceOutsideGap(*gap, point);

        if (away < closest)
        {
            closest = away;
            nearest = gap;
        }
    }

    return *nearest;
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

struct Clip final
{
    float entry = 0.f;
    float exit = 1.f;
    bool inside = true;
    bool missed = false;
};

void clipAgainstSide(Clip& clip, const HalfSpace& side, Vec3 from, Vec3 direction)
{
    auto room = side.offset - dot(side.normal, from);
    auto closing = dot(side.normal, direction);
    clip.inside = clip.inside && room >= 0.f;

    if (closing == 0.f)
    {
        clip.missed = room < 0.f;
        return;
    }

    auto crossing = room / closing;

    if (closing < 0.f)
        clip.entry = std::max(clip.entry, crossing);
    else
        clip.exit = std::min(clip.exit, crossing);
}

// Cyrus-Beck clipping; nothing when the segment misses or starts inside.
std::optional<float> entryInto(const Block& block, Vec3 from, Vec3 to)
{
    auto direction = to - from;
    auto clip = Clip {};

    for (const auto& side: sidesOf(block))
    {
        clipAgainstSide(clip, side, from, direction);

        if (clip.missed)
            return std::nullopt;
    }

    if (clip.inside || clip.entry > clip.exit)
        return std::nullopt;

    return clip.entry;
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
    constexpr auto pushPasses = 2;

    for (auto pass = 0; pass < pushPasses; ++pass)
    {
        for (const auto& collider: colliders)
            point = pushedOutOf(collider, {{point, feet}, radius});

        for (const auto& mover: movers)
            point = pushedOutOf(mover.collider, {{point, feet}, radius});

        for (const auto& block: blocks)
            point = pushedOutOf(block, {{point, feet}, radius});
    }

    return keptInGap(point, radius, feet);
}

Vec2 Level::keptInGap(Vec2 point, float radius, float feet) const
{
    if (gaps.empty() || feet + maxStepUpHeight >= 0.f)
        return point;

    if (overGap(point))
        return point;

    const auto& nearest = findNearestGap(gaps, point);
    auto room = Vec2 {std::max(nearest.half.x - radius, 0.f),
                      std::max(nearest.half.y - radius, 0.f)};
    return {
        std::clamp(point.x, nearest.center.x - room.x, nearest.center.x + room.x),
        std::clamp(point.y, nearest.center.y - room.y, nearest.center.y + room.y)};
}

float Level::floorAt(Vec2 point, float feet) const
{
    auto floor = groundAt(point);
    auto stance = Stance {point, feet};

    for (const auto& collider: colliders)
        floor = std::max(floor, getFloorOfCollider(collider, stance));

    for (const auto& mover: movers)
        floor = std::max(floor, getFloorOfCollider(mover.collider, stance));

    for (const auto& block: blocks)
        floor = std::max(floor, getFloorOfBlock(block, stance));

    return floor;
}

float Level::ceilingAt(Vec2 point, float feet) const
{
    auto ceiling = std::numeric_limits<float>::infinity();

    for (const auto& block: blocks)
        if (block.contains(point) && passesUnder(block, {point, feet}))
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
                           return standsOn(mover.collider, {point, feet})
                                  && std::abs(mover.collider.top - feet)
                                         < onMoverTolerance;
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
