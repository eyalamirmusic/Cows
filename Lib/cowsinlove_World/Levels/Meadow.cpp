#include "Levels/Meadow.h"
#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Props/Crate.h"
#include "Props/Hedge.h"
#include "Props/Props.h"
#include "Props/Rock.h"
#include "Props/Tree.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto arenaReach = 86.f;
constexpr auto clearing = 9.f;
constexpr auto groveCount = 9;
constexpr auto hedgeCount = 16;
constexpr auto baleClusterCount = 7;
constexpr auto rockCount = 28;
constexpr auto barnCount = 5;
constexpr auto crateCount = 12;
constexpr auto barnSpacing = 26.f;
constexpr auto nearestHideout = 50.f;
constexpr auto furthestHideout = 75.f;
constexpr auto structureMargin = 2.5f;
constexpr auto hidingKinds = 5;
constexpr auto nearestHiding = 25.f;
constexpr auto hidingRoom = 1.2f;
constexpr auto meadowRoom = 3.f;
constexpr auto meadowTries = 24;
constexpr auto groveRing = 3.5f;
constexpr auto groveReach = 5.f;
constexpr auto groveTrees = 2;
constexpr auto hedgeShelter = 2.6f;

struct Layout final
{
    explicit Layout(std::uint32_t seed)
        : random(seed)
    {
    }

    float unit() { return randomUnit(random); }

    float between(float from, float to) { return randomBetween(random, from, to); }

    Vec2 spot()
    {
        while (true)
        {
            auto point = Vec2 {between(-arenaReach, arenaReach),
                               between(-arenaReach, arenaReach)};

            if (length(point) > clearing + 4.f)
                return point;
        }
    }

    template <std::size_t Count>
    std::uint32_t pick(const std::uint32_t (&colors)[Count])
    {
        return randomPick(random, colors);
    }

    int index(int count)
    {
        return std::min((int) (unit() * (float) count), count - 1);
    }

    void addHiding(Level::Hiding kind, Vec3 at)
    {
        hidings[(std::size_t) kind].add(at);
    }

    std::mt19937 random;
    std::array<Vector<Vec3>, hidingKinds> hidings;
    Vector<Vec2> trees;
};

bool inClearing(Vec2 point)
{
    return length(point) < clearing;
}

void plantTree(Level& level, Layout& layout, Vec2 at)
{
    if (inClearing(at) || !level.isFree(at, structureMargin))
        return;

    addTree(level, layout.random, at);
    layout.trees.add(at);
}

void addGrove(Level& level, Layout& layout)
{
    auto center = layout.spot();
    auto count = 5 + (int) (layout.unit() * 6.f);

    for (auto tree = 0; tree < count; ++tree)
    {
        auto angle = layout.between(0.f, twoPi);
        auto reach = layout.between(2.f, 11.f);
        plantTree(level,
                  layout,
                  center + Vec2 {std::cos(angle) * reach, std::sin(angle) * reach});
    }

    layout.addHiding(Level::Hiding::Grove, {center.x, 0.f, center.y});

    for (auto side = 0; side < 6; ++side)
    {
        auto angle = twoPi * (float) side / 6.f;
        auto at = center + Vec2 {std::cos(angle), std::sin(angle)} * groveRing;
        layout.addHiding(Level::Hiding::Grove, {at.x, 0.f, at.y});
    }
}

void addHedgerow(Level& level, Layout& layout)
{
    auto start = layout.spot();
    auto heading = layout.unit() < 0.7f
                       ? halfPi * (float) (int) (layout.unit() * 4.f)
                             + layout.between(-0.15f, 0.15f)
                       : layout.between(0.f, twoPi);
    auto direction = Vec2 {std::cos(heading), -std::sin(heading)};
    auto pieces = 6 + (int) (layout.unit() * 10.f);
    auto gap = 2 + (int) (layout.unit() * (float) (pieces - 4));
    auto color = layout.pick(hedgeColors);

    for (auto piece = 0; piece < pieces; ++piece)
    {
        if (piece == gap || piece == gap + 1)
            continue;

        auto at = start + direction * (2.2f * (float) piece);

        if (inClearing(at) || !level.isFree(at, structureMargin)
            || std::abs(at.x) > arenaReach + 4.f
            || std::abs(at.y) > arenaReach + 4.f)
            continue;

        addHedgePiece(level, layout.random, at, heading, color);

        auto across = Vec2 {-direction.y, direction.x};
        auto behind =
            at + across * (dot(across, at) < 0.f ? -hedgeShelter : hedgeShelter);
        layout.addHiding(Level::Hiding::Hedge, {behind.x, 0.f, behind.y});
    }
}

void addBales(Level& level, Layout& layout)
{
    auto center = layout.spot();
    auto count = 3 + (int) (layout.unit() * 4.f);

    for (auto bale = 0; bale < count; ++bale)
    {
        auto at =
            center + Vec2 {layout.between(-4.f, 4.f), layout.between(-4.f, 4.f)};

        if (inClearing(at) || !level.isFree(at, structureMargin))
            continue;

        addBale(level, layout.random, at);
        layout.addHiding(Level::Hiding::Stack, {at.x, baleRadius * 2.f, at.y});
    }
}

void placeRock(Level& level, Layout& layout)
{
    auto at = layout.spot();

    if (!level.isFree(at, structureMargin))
        return;

    addRock(level, layout.random, at);
}

Vec2 hideoutSpot(Layout& layout)
{
    while (true)
    {
        auto angle = layout.between(0.f, twoPi);
        auto reach = layout.between(nearestHideout, furthestHideout);
        auto at = Vec2 {std::cos(angle) * reach, std::sin(angle) * reach};

        if (std::abs(at.x) < arenaReach - 14.f && std::abs(at.y) < arenaReach - 14.f)
            return at;
    }
}

void addBarns(Level& level, Layout& layout)
{
    auto spots = Vector<Vec2> {};
    spots.add(hideoutSpot(layout));

    for (auto attempt = 0; attempt < 400 && spots.size() < barnCount; ++attempt)
    {
        auto at = Vec2 {layout.between(-arenaReach + 14.f, arenaReach - 14.f),
                        layout.between(-arenaReach + 14.f, arenaReach - 14.f)};
        auto roomy = length(at) > 25.f
                     && std::all_of(spots.begin(),
                                    spots.end(),
                                    [&](Vec2 other)
                                    { return distance(at, other) > barnSpacing; });

        if (roomy)
            spots.add(at);
    }

    layout.addHiding(Level::Hiding::Roof, {spots[0].x, barnRoofHeight, spots[0].y});

    for (const auto& spot: spots)
        addBarn(level, spot, (int) (layout.unit() * 4.f) % 4);
}

void addCrates(Level& level, Layout& layout)
{
    for (auto crate = 0; crate < crateCount; ++crate)
    {
        auto at = layout.spot();

        if (!level.isFree(at, structureMargin + 1.f))
            continue;

        layout.addHiding(Level::Hiding::Stack, addCrate(level, layout.random, at));
    }
}

bool hasRoom(const Level& level, Vec3 spot, float margin)
{
    auto at = Vec2 {spot.x, spot.z};
    auto overhead = spot.y + stepUp;

    if (length(at) < nearestHiding || std::abs(at.x) > arenaReach
        || std::abs(at.y) > arenaReach
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

void addMeadowHidings(const Level& level, Layout& layout)
{
    for (auto attempt = 0; attempt < meadowTries; ++attempt)
    {
        auto at = layout.spot();
        auto spot = Vec3 {at.x, 0.f, at.y};

        if (hasRoom(level, spot, meadowRoom))
            layout.addHiding(Level::Hiding::Meadow, spot);
    }
}

void chooseHideout(Level& level, Layout& layout)
{
    addMeadowHidings(level, layout);

    auto kinds = Vector<int> {};

    for (auto kind = 0; kind < hidingKinds; ++kind)
    {
        auto& spots = layout.hidings[(std::size_t) kind];
        auto grove = kind == (int) Level::Hiding::Grove;
        auto roof = kind == (int) Level::Hiding::Roof;

        auto unfit = [&](Vec3 spot)
        {
            return !roof
                   && (!hasRoom(level, spot, hidingRoom)
                       || (grove && !amongTrees(layout, spot)));
        };
        spots.erase(std::remove_if(spots.begin(), spots.end(), unfit), spots.end());

        if (!spots.empty())
            kinds.add(kind);
    }

    auto kind = kinds[layout.index(kinds.size())];
    auto& spots = layout.hidings[(std::size_t) kind];

    level.hiding = (Level::Hiding) kind;
    level.hideout = spots[layout.index(spots.size())];
}
} // namespace

Level makeMeadow(std::uint32_t seed)
{
    auto level = Level {};
    auto layout = Layout {seed * 2654435761u + 17u};

    addBarns(level, layout);
    addCrates(level, layout);

    for (auto grove = 0; grove < groveCount; ++grove)
        addGrove(level, layout);

    for (auto hedge = 0; hedge < hedgeCount; ++hedge)
        addHedgerow(level, layout);

    for (auto cluster = 0; cluster < baleClusterCount; ++cluster)
        addBales(level, layout);

    for (auto rock = 0; rock < rockCount; ++rock)
        placeRock(level, layout);

    chooseHideout(level, layout);
    return level;
}
} // namespace Cows
