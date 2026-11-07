#include "Levels/Layout.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto hidingRoom = 1.2f;
constexpr auto meadowRoom = 3.f;
constexpr auto meadowTries = 24;
constexpr auto groveReach = 5.f;
constexpr auto groveTrees = 2;
constexpr auto floorTolerance = 0.01f;

void addMeadowHidings(const Level& level, Layout& layout, const Region& region)
{
    for (auto attempt = 0; attempt < meadowTries; ++attempt)
    {
        auto at = layout.spot(region);
        auto spot = Vec3 {at.x, 0.f, at.y};

        if (hasRoom(level, spot, meadowRoom))
            layout.addHiding(Level::Hiding::Meadow, spot);
    }
}

struct Clearance final
{
    Vec2 at;
    float overhead = 0.f;
    float margin = 0.f;
};

bool isBlockedBy(const Collider& collider, const Clearance& clearance)
{
    return collider.top > clearance.overhead
           && distance(clearance.at, collider.center)
                  < collider.radius + clearance.margin;
}

bool isWalledBy(const Block& block, const Clearance& clearance)
{
    return block.top > clearance.overhead
           && block.contains(clearance.at, clearance.margin);
}

bool isFarFromStartInsideArena(const Level& level, Vec2 at)
{
    if (distance(at, {level.start.x, level.start.z}) < nearestHiding)
        return false;

    auto isOutsideArena = std::abs(at.x) > arenaReach || std::abs(at.y) > arenaReach;
    return !isOutsideArena;
}

bool standsOnFloor(const Level& level, Vec3 spot)
{
    auto floorHeight = level.floorAt({spot.x, spot.z}, spot.y);
    auto isOffFloor = std::abs(floorHeight - spot.y) > floorTolerance;
    return !isOffFloor;
}

bool isInBarn(Level::Hiding kind)
{
    return kind == Level::Hiding::Roof || kind == Level::Hiding::Loft;
}

bool isUnfitOutsideBarn(const Level& level,
                        const Layout& layout,
                        Level::Hiding kind,
                        const HidingSpot& spot)
{
    if (!hasRoom(level, spot.at, hidingRoom))
        return true;

    return kind == Level::Hiding::Grove && !amongTrees(layout, spot.at);
}

void removeUnfitHidingSpots(const Level& level,
                            Layout& layout,
                            const Region& region,
                            Level::Hiding kind)
{
    auto& spots = layout.hidings[(std::size_t) kind];
    auto unfit = [&](const HidingSpot& spot)
    {
        if (!region.contains({spot.at.x, spot.at.z}))
            return true;

        return !isInBarn(kind) && isUnfitOutsideBarn(level, layout, kind, spot);
    };
    spots.erase(std::remove_if(spots.begin(), spots.end(), unfit), spots.end());
}

void hideInMiddleOfMeadow(Level& level, const Region& region)
{
    auto center = region.center();
    level.hiding = Level::Hiding::Meadow;
    level.hideout = {center.x, level.floorAt(center, 0.f), center.y};
    level.hideoutHeading.reset();
}
} // namespace

Layout::Layout(std::uint32_t seed)
    : random(seed)
{
}

float Layout::unit()
{
    return randomUnit(random);
}

float Layout::between(float from, float to)
{
    return randomBetween(random, from, to);
}

Vec2 Layout::spot(const Region& region)
{
    auto area = region.inset(arenaInset);

    while (true)
    {
        auto point =
            Vec2 {between(area.min.x, area.max.x), between(area.min.y, area.max.y)};

        if (distance(point, home) > clearing + 4.f)
            return point;
    }
}

int Layout::index(int count)
{
    return std::min((int) (unit() * (float) count), count - 1);
}

int Layout::share(int count, const Region& region) const
{
    return (int) std::lround((float) count * region.area() / Region::arena().area());
}

void Layout::addHiding(Level::Hiding kind, Vec3 at, std::optional<float> heading)
{
    hidings[(std::size_t) kind].add({at, heading});
}

bool Layout::inClearing(Vec2 point) const
{
    return distance(point, home) < clearing;
}

bool Layout::onPath(Vec2 point, float margin) const
{
    return !path.isEmpty() && path.contains(point, margin);
}

bool hasRoom(const Level& level, Vec3 spot, float margin)
{
    auto at = Vec2 {spot.x, spot.z};

    if (!isFarFromStartInsideArena(level, at) || !standsOnFloor(level, spot))
        return false;

    auto clearance = Clearance {at, spot.y + maxStepUpHeight, margin};
    auto blocked = [&](const Collider& collider)
    { return isBlockedBy(collider, clearance); };
    auto walled = [&](const Block& block) { return isWalledBy(block, clearance); };

    return std::none_of(level.colliders.begin(), level.colliders.end(), blocked)
           && std::none_of(level.blocks.begin(), level.blocks.end(), walled);
}

bool amongTrees(const Layout& layout, Vec3 spot)
{
    auto near = std::count_if(
        layout.trees.begin(),
        layout.trees.end(),
        [&](Vec2 tree) { return distance(tree, {spot.x, spot.z}) < groveReach; });
    return near >= groveTrees;
}

void chooseHideout(Level& level, Layout& layout, const Region& region)
{
    addMeadowHidings(level, layout, region);

    auto kinds = Vector<int> {};

    for (auto kind = 0; kind < hidingKinds; ++kind)
    {
        auto& spots = layout.hidings[(std::size_t) kind];
        removeUnfitHidingSpots(level, layout, region, (Level::Hiding) kind);

        if (!spots.empty())
            kinds.add(kind);
    }

    if (kinds.empty())
    {
        hideInMiddleOfMeadow(level, region);
        return;
    }

    auto kind = kinds[layout.index(kinds.size())];
    auto& spots = layout.hidings[(std::size_t) kind];

    level.hiding = (Level::Hiding) kind;
    const auto& spot = spots[layout.index(spots.size())];
    level.hideout = spot.at;
    level.hideoutHeading = spot.heading;
}
} // namespace Cows
