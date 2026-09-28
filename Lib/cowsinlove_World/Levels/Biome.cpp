#include "Levels/Biome.h"
#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Props/Crate.h"
#include "Props/Hedge.h"
#include "Props/Props.h"
#include "Props/Rock.h"
#include "Props/Tree.h"

#include <algorithm>
#include <optional>
#include <cmath>
#include <random>

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
constexpr auto barnRoom = 13.f;
constexpr auto barnSpacing = 26.f;
constexpr auto nearestHideout = 50.f;
constexpr auto furthestHideout = 75.f;
constexpr auto structureMargin = 2.5f;
constexpr auto barnInset = 28.f;
constexpr auto hideoutTries = 1000;
constexpr auto groveRing = 3.5f;
constexpr auto hedgeShelter = 2.6f;

void plantTree(Level& level, Layout& layout, Vec2 at)
{
    if (layout.inClearing(at) || !level.isFree(at, structureMargin)
        || layout.onPath(at, treeRoom))
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

void addHedgerow(Level& level, Layout& layout, const Region& region)
{
    auto start = layout.spot(region);
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

        if (layout.inClearing(at) || !level.isFree(at, structureMargin)
            || !region.inset(arenaInset - 4.f).contains(at)
            || layout.onPath(at, hedgeRoom))
            continue;

        addHedgePiece(level, layout.random, at, heading, color);

        auto across = Vec2 {-direction.y, direction.x};
        auto behind = at
                      + across
                            * (dot(across, at - layout.home) < 0.f ? -hedgeShelter
                                                                   : hedgeShelter);
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

        if (layout.inClearing(at) || !level.isFree(at, structureMargin)
            || layout.onPath(at, baleRoom))
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

        if (area.holds(at) && !layout.onPath(at, barnRoom))
            return at;
    }

    return std::nullopt;
}

void addBarns(Level& level, Layout& layout, const Region& region, int barns)
{
    auto area = region.inset(barnInset);

    if (area.isEmpty())
        return;
    auto count = layout.share(barns, region);
    auto spots = Vector<Vec2> {};
    auto roof = hideoutSpot(layout, area);

    if (roof)
        spots.add(*roof);

    for (auto attempt = 0; attempt < 400 && spots.size() < count; ++attempt)
    {
        auto at = Vec2 {layout.between(area.min.x, area.max.x),
                        layout.between(area.min.y, area.max.y)};
        auto roomy = distance(at, layout.home) > 25.f && !layout.onPath(at, barnRoom)
                     && std::all_of(spots.begin(),
                                    spots.end(),
                                    [&](Vec2 other)
                                    { return distance(at, other) > barnSpacing; });

        if (roomy)
            spots.add(at);
    }

    if (roof)
        layout.addHiding(Level::Hiding::Roof, {roof->x, barnRoofHeight, roof->y});

    for (const auto& spot: spots)
        addBarn(level, spot, (int) (layout.unit() * 4.f) % 4);
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
