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

void Layout::addHiding(Level::Hiding kind, Vec3 at)
{
    hidings[(std::size_t) kind].add(at);
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
    auto overhead = spot.y + stepUp;

    if (distance(at, {level.start.x, level.start.z}) < nearestHiding
        || std::abs(at.x) > arenaReach || std::abs(at.y) > arenaReach
        || std::abs(level.floorAt(at, spot.y) - spot.y) > 0.01f)
        return false;

    auto blocked = [&](const Collider& collider)
    {
        return collider.top > overhead
               && distance(at, collider.center) < collider.radius + margin;
    };

    auto walled = [&](const Block& block)
    { return block.top > overhead && block.contains(at, margin); };

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
        auto grove = kind == (int) Level::Hiding::Grove;
        auto roof = kind == (int) Level::Hiding::Roof;

        auto unfit = [&](Vec3 spot)
        {
            return !region.contains({spot.x, spot.z})
                   || (!roof
                       && (!hasRoom(level, spot, hidingRoom)
                           || (grove && !amongTrees(layout, spot))));
        };
        spots.erase(std::remove_if(spots.begin(), spots.end(), unfit), spots.end());

        if (!spots.empty())
            kinds.add(kind);
    }

    if (kinds.empty())
    {
        auto center = region.center();
        level.hiding = Level::Hiding::Meadow;
        level.hideout = {center.x, level.floorAt(center, 0.f), center.y};
        return;
    }

    auto kind = kinds[layout.index(kinds.size())];
    auto& spots = layout.hidings[(std::size_t) kind];

    level.hiding = (Level::Hiding) kind;
    level.hideout = spots[layout.index(spots.size())];
}
} // namespace Cows
