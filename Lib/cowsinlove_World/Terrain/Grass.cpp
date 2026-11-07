#include "Terrain/Grass.h"

#include "Props/Props.h"
#include "Render/Frustum.h"

#include <algorithm>

#include <cmath>
#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto bladeColumns = 3;
constexpr auto rimMargin = 0.15f;

bool doesTileOverlapGap(const Gap& gap, Vec2 corner)
{
    auto near = gap.center - gap.half - Vec2 {rimMargin, rimMargin};
    auto far = gap.center + gap.half + Vec2 {rimMargin, rimMargin};
    return corner.x < far.x && corner.x + meadowTile > near.x && corner.y < far.y
           && corner.y + meadowTile > near.y;
}

bool crossesGap(const Vector<Gap>& gaps, Vec2 corner)
{
    return std::any_of(gaps.begin(),
                       gaps.end(),
                       [&](const Gap& gap)
                       { return doesTileOverlapGap(gap, corner); });
}

struct BladeGridSpot final
{
    int segment = 0;
    int column = 0;
};

std::uint16_t getBladeVertexIndex(BladeGridSpot spot)
{
    return (std::uint16_t) (spot.segment * bladeColumns + spot.column);
}

void addBladeSegmentQuads(Blade& blade, int bladeSegments)
{
    for (auto segment = 0; segment + 1 < bladeSegments; ++segment)
    {
        for (auto column = 0; column + 1 < bladeColumns; ++column)
        {
            auto a = getBladeVertexIndex({segment, column});
            auto b = getBladeVertexIndex({segment, column + 1});
            auto c = getBladeVertexIndex({segment + 1, column + 1});
            auto d = getBladeVertexIndex({segment + 1, column});
            blade.indices.add({a, b, c, a, c, d});
        }
    }
}

float distanceToTile(Vec2 point, Vec2 corner)
{
    auto nearest = Vec2 {std::clamp(point.x, corner.x, corner.x + meadowTile),
                         std::clamp(point.y, corner.y, corner.y + meadowTile)};
    return length(point - nearest);
}

float shareAt(const GrassDensity& density, float distance)
{
    if (distance <= density.thinFrom)
        return density.nearShare;

    if (distance >= density.thinTo)
        return density.farShare;

    auto along = (distance - density.thinFrom) / (density.thinTo - density.thinFrom);
    return density.nearShare + (density.farShare - density.nearShare) * along;
}

bool isNearGap(const Gap& gap, Vec2 point)
{
    auto offset = absolute(point - gap.center);
    return offset.x < gap.half.x + rimMargin && offset.y < gap.half.y + rimMargin;
}

bool nearGap(const Vector<Gap>& gaps, Vec2 point)
{
    return std::any_of(gaps.begin(),
                       gaps.end(),
                       [&](const Gap& gap) { return isNearGap(gap, point); });
}

bool isTileInView(const Mat4& viewProjection, Vec2 at)
{
    auto low = Vec3 {at.x - bladeReach, 0.f, at.y - bladeReach};
    auto high = Vec3 {
        at.x + meadowTile + bladeReach, bladeReach, at.y + meadowTile + bladeReach};
    return boxInView(viewProjection, low, high);
}
} // namespace

Blade makeBlade(int bladeSegments)
{
    auto blade = Blade {};

    for (auto segment = 0; segment < bladeSegments; ++segment)
    {
        auto height = (float) segment / (float) bladeSegments;

        for (auto column = 0; column < bladeColumns; ++column)
            blade.vertices.add({{(float) column - 1.f, height}});
    }

    blade.vertices.add({{0.f, 1.f}});

    addBladeSegmentQuads(blade, bladeSegments);

    auto tip = (std::uint16_t) (bladeSegments * bladeColumns);

    for (auto column = 0; column + 1 < bladeColumns; ++column)
        blade.indices.add({getBladeVertexIndex({bladeSegments - 1, column}),
                           getBladeVertexIndex({bladeSegments - 1, column + 1}),
                           tip});

    return blade;
}

Vector<BladeInstance> makeGrassTile()
{
    auto blades = Vector<BladeInstance> {};
    blades.reserve(bladeCount);

    auto random = std::mt19937 {1994u};

    while (blades.size() < bladeCount)
    {
        auto x = meadowTile * randomUnit(random);
        auto z = meadowTile * randomUnit(random);

        auto height = 0.2f + 0.2f * randomUnit(random);
        auto width = 0.018f + 0.012f * randomUnit(random);

        auto facing = twoPi * randomUnit(random);
        auto phase = twoPi * randomUnit(random);
        auto shade = randomUnit(random);
        auto lean = randomUnit(random) - 0.5f;

        auto twist = 1.8f * (randomUnit(random) - 0.5f);
        auto fold = 0.35f + 0.65f * randomUnit(random);
        auto hue = randomUnit(random);
        auto seed = randomUnit(random);

        blades.add(BladeInstance {{x, z, facing, height},
                                  {width, phase, shade, lean},
                                  {twist, fold, hue, seed}});
    }

    return blades;
}

Vector<BladeInstance>
    cutTile(const Vector<BladeInstance>& tile, Vec2 corner, const Vector<Gap>& gaps)
{
    auto blades = Vector<BladeInstance> {};

    for (const auto& blade: tile)
    {
        auto at = corner + Vec2 {blade.placement.x, blade.placement.y};

        if (!nearGap(gaps, at))
            blades.add(blade);
    }

    return blades;
}

Vector<GrassDraw> planGrass(Vec2 focus,
                            const Mat4& viewProjection,
                            const Vec3& eye,
                            const GrassDensity& density)
{
    auto corner = Vec2 {std::round(focus.x / meadowTile) * meadowTile,
                        std::round(focus.y / meadowTile) * meadowTile};
    auto ground = Vec2 {eye.x, eye.z};

    struct Placed
    {
        GrassDraw draw;
        float distance = 0.f;
    };

    auto placed = Vector<Placed> {};

    for (auto x = -density.tilesAround; x < density.tilesAround; ++x)
        for (auto z = -density.tilesAround; z < density.tilesAround; ++z)
        {
            auto at = corner + Vec2 {(float) x * meadowTile, (float) z * meadowTile};

            if (!isTileInView(viewProjection, at))
                continue;

            auto distance = distanceToTile(ground, at);
            placed.add({{at, shareAt(density, distance)}, distance});
        }

    std::stable_sort(placed.begin(),
                     placed.end(),
                     [](const Placed& a, const Placed& b)
                     { return a.distance < b.distance; });

    auto draws = Vector<GrassDraw> {};

    for (const auto& each: placed)
        draws.add(each.draw);

    return draws;
}

int bladesToDraw(int available, float share)
{
    if (available <= 0 || share <= 0.f)
        return 0;

    return std::clamp((int) std::lround((float) available * share), 1, available);
}

void GrassField::layOver(const Level& level)
{
    gaps = level.gaps;

    for (const auto& clearing: level.clearings)
        gaps.add(Gap {clearing.center, clearing.half});

    cutTiles.clear();
}

const Vector<BladeInstance>& GrassField::tileAt(Vec2 corner)
{
    if (!crossesGap(gaps, corner))
        return tile;

    auto key = std::make_pair((int) std::lround(corner.x / meadowTile),
                              (int) std::lround(corner.y / meadowTile));
    auto found = cutTiles.find(key);

    if (found == cutTiles.end())
        found = cutTiles.emplace(key, cutTile(tile, corner, gaps)).first;

    return found->second;
}
} // namespace Cows
