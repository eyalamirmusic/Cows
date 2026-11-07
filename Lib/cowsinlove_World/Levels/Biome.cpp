#include "Levels/Biome.h"
#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Props/Crate.h"
#include "Props/Hedge.h"
#include "Props/Rock.h"
#include "Props/Tree.h"

#include <algorithm>
#include <optional>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto treeRoom = 1.f;
constexpr auto hedgeRoom = 2.f;
constexpr auto baleRoom = 1.5f;
constexpr auto rockRoom = 1.5f;
constexpr auto crateRoom = 3.f;
constexpr auto nearestHideout = 50.f;
constexpr auto furthestHideout = 75.f;
constexpr auto structureMargin = 2.5f;
constexpr auto barnInset = 28.f;
constexpr auto hideoutTries = 1000;
constexpr auto groveRing = 3.5f;
constexpr auto hedgeShelter = 2.6f;
constexpr auto barnTries = 400;
constexpr auto barnDistanceFromHome = 25.f;
constexpr auto alignedHedgerowChance = 0.7f;
constexpr auto hedgerowWobble = 0.15f;
constexpr auto hedgeSpacing = 2.2f;
constexpr auto hedgerowRegionSlack = 4.f;

bool isOpenGround(const Level& level, const Layout& layout, Vec2 at, float room)
{
    return !layout.inClearing(at) && level.isFree(at, structureMargin)
           && !layout.onPath(at, room);
}

void plantTree(Level& level, Layout& layout, Vec2 at)
{
    if (!isOpenGround(level, layout, at, treeRoom))
        return;

    addTree(level, layout.random, at);
    layout.trees.add(at);
}

void addGrove(Level& level, Layout& layout, const Region& region)
{
    auto center = layout.spot(region);
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

float chooseHedgerowHeading(Layout& layout)
{
    auto isAligned = layout.unit() < alignedHedgerowChance;

    if (!isAligned)
        return layout.between(0.f, twoPi);

    auto quarter = (float) (int) (layout.unit() * 4.f);
    return halfPi * quarter + layout.between(-hedgerowWobble, hedgerowWobble);
}

bool canPlaceHedgePiece(const Level& level,
                        const Layout& layout,
                        const Region& region,
                        Vec2 at)
{
    return !layout.inClearing(at) && level.isFree(at, structureMargin)
           && region.inset(arenaInset - hedgerowRegionSlack).contains(at)
           && !layout.onPath(at, hedgeRoom);
}

Vec2 getSpotBehindHedgeAwayFromHome(const Layout& layout, Vec2 at, Vec2 direction)
{
    auto across = Vec2 {-direction.y, direction.x};
    auto isHomeSide = dot(across, at - layout.home) < 0.f;

    if (isHomeSide)
        return at + across * -hedgeShelter;

    return at + across * hedgeShelter;
}

void addHedgerow(Level& level, Layout& layout, const Region& region)
{
    auto start = layout.spot(region);
    auto heading = chooseHedgerowHeading(layout);
    auto direction = Vec2 {std::cos(heading), -std::sin(heading)};
    auto pieces = 6 + (int) (layout.unit() * 10.f);
    auto gap = 2 + (int) (layout.unit() * (float) (pieces - 4));
    auto color = layout.pick(hedgeColors);

    for (auto piece = 0; piece < pieces; ++piece)
    {
        if (piece == gap || piece == gap + 1)
            continue;

        auto at = start + direction * (hedgeSpacing * (float) piece);

        if (!canPlaceHedgePiece(level, layout, region, at))
            continue;

        addHedgePiece(level, layout.random, {at, heading, color});

        auto behind = getSpotBehindHedgeAwayFromHome(layout, at, direction);
        layout.addHiding(Level::Hiding::Hedge, {behind.x, 0.f, behind.y});
    }
}

void addBales(Level& level, Layout& layout, const Region& region)
{
    auto center = layout.spot(region);
    auto count = 3 + (int) (layout.unit() * 4.f);

    for (auto bale = 0; bale < count; ++bale)
    {
        auto at =
            center + Vec2 {layout.between(-4.f, 4.f), layout.between(-4.f, 4.f)};

        if (!isOpenGround(level, layout, at, baleRoom))
            continue;

        addBale(level, layout.random, at);
        layout.addHiding(Level::Hiding::Stack, {at.x, baleRadius * 2.f, at.y});
    }
}

void placeRock(Level& level, Layout& layout, const Region& region)
{
    auto at = layout.spot(region);

    if (!level.isFree(at, structureMargin) || layout.onPath(at, rockRoom))
        return;

    addRock(level, layout.random, at);
}

std::optional<Vec2> hideoutSpot(Layout& layout, const Region& area)
{
    for (auto attempt = 0; attempt < hideoutTries; ++attempt)
    {
        auto angle = layout.between(0.f, twoPi);
        auto reach = layout.between(nearestHideout, furthestHideout);
        auto at =
            layout.home + Vec2 {std::cos(angle) * reach, std::sin(angle) * reach};

        if (area.holds(at) && !layout.onPath(at, barnReach()))
            return at;
    }

    return std::nullopt;
}

void addBarnHidings(Layout& layout, Vec2 center, int quarterTurns)
{
    auto spots = barnSpots(center, quarterTurns);
    layout.addHiding(Level::Hiding::Roof, spots.roof.at, spots.roof.heading);
    layout.addHiding(Level::Hiding::Loft, spots.loft.at, spots.loft.heading);
}

bool isRoomForBarn(const Layout& layout, const Vector<Vec2>& spots, Vec2 at)
{
    auto isFarFromHome = distance(at, layout.home) > barnDistanceFromHome;

    if (!isFarFromHome || layout.onPath(at, barnReach()))
        return false;

    return std::all_of(spots.begin(),
                       spots.end(),
                       [&](Vec2 other)
                       { return distance(at, other) > 2.f * barnReach(); });
}

void addRandomBarnSpots(Layout& layout,
                        const Region& area,
                        Vector<Vec2>& spots,
                        int count)
{
    for (auto attempt = 0; attempt < barnTries && spots.size() < count; ++attempt)
    {
        auto at = Vec2 {layout.between(area.min.x, area.max.x),
                        layout.between(area.min.y, area.max.y)};

        if (isRoomForBarn(layout, spots, at))
            spots.add(at);
    }
}

void addBarns(Level& level, Layout& layout, const Region& region, int barns)
{
    auto area = region.inset(barnInset);

    if (area.isEmpty())
        return;

    auto count = layout.share(barns, region);
    auto spots = Vector<Vec2> {};
    auto hideout = hideoutSpot(layout, area);

    if (hideout)
        spots.add(*hideout);

    addRandomBarnSpots(layout, area, spots, count);

    for (auto index = 0; index < spots.size(); ++index)
    {
        auto quarterTurns = (int) (layout.unit() * 4.f) % 4;

        if (hideout && index == 0)
            addBarnHidings(layout, spots[index], quarterTurns);

        addBarn(level, spots[index], quarterTurns);
    }
}

void addCrates(Level& level, Layout& layout, const Region& region, int crates)
{
    for (auto crate = 0; crate < layout.share(crates, region); ++crate)
    {
        auto at = layout.spot(region);

        if (!level.isFree(at, structureMargin + 1.f) || layout.onPath(at, crateRoom))
            continue;

        layout.addHiding(Level::Hiding::Stack, addCrate(level, layout.random, at));
    }
}
} // namespace

Biome meadowBiome()
{
    return {};
}

void populate(Level& level, Layout& layout, Region region, const Biome& biome)
{
    addBarns(level, layout, region, biome.barns);
    addCrates(level, layout, region, biome.crates);

    for (auto grove = 0; grove < layout.share(biome.groves, region); ++grove)
        addGrove(level, layout, region);

    for (auto hedge = 0; hedge < layout.share(biome.hedgerows, region); ++hedge)
        addHedgerow(level, layout, region);

    for (auto cluster = 0; cluster < layout.share(biome.baleClusters, region);
         ++cluster)
        addBales(level, layout, region);

    for (auto rock = 0; rock < layout.share(biome.rocks, region); ++rock)
        placeRock(level, layout, region);
}
} // namespace Cows
