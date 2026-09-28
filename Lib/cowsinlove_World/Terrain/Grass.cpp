#include "Terrain/Grass.h"

#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto bladeSegments = 5;
constexpr auto bladeColumns = 3;
} // namespace

Blade makeBlade()
{
    auto blade = Blade {};

    for (auto segment = 0; segment < bladeSegments; ++segment)
    {
        auto height = (float) segment / (float) bladeSegments;

        for (auto column = 0; column < bladeColumns; ++column)
            blade.vertices.add({{(float) column - 1.f, height}});
    }

    blade.vertices.add({{0.f, 1.f}});

    auto at = [](int segment, int column)
    { return (std::uint16_t) (segment * bladeColumns + column); };

    for (auto segment = 0; segment + 1 < bladeSegments; ++segment)
        for (auto column = 0; column + 1 < bladeColumns; ++column)
        {
            auto a = at(segment, column);
            auto b = at(segment, column + 1);
            auto c = at(segment + 1, column + 1);
            auto d = at(segment + 1, column);
            blade.indices.add({a, b, c, a, c, d});
        }

    auto tip = (std::uint16_t) (bladeSegments * bladeColumns);

    for (auto column = 0; column + 1 < bladeColumns; ++column)
        blade.indices.add(
            {at(bladeSegments - 1, column), at(bladeSegments - 1, column + 1), tip});

    return blade;
}

Vector<BladeInstance> makeGrassTile()
{
    auto blades = Vector<BladeInstance> {};
    blades.reserve(bladeCount);

    auto random = std::mt19937 {1994u};
    auto unit = std::uniform_real_distribution<float> {0.f, 1.f};

    while (blades.size() < bladeCount)
    {
        auto x = meadowTile * unit(random);
        auto z = meadowTile * unit(random);

        auto height = 0.2f + 0.2f * unit(random);
        auto width = 0.018f + 0.012f * unit(random);

        auto facing = twoPi * unit(random);
        auto phase = twoPi * unit(random);
        auto shade = unit(random);
        auto lean = unit(random) - 0.5f;

        auto twist = 1.8f * (unit(random) - 0.5f);
        auto fold = 0.35f + 0.65f * unit(random);
        auto hue = unit(random);
        auto seed = unit(random);

        blades.add(BladeInstance {{x, z, facing, height},
                                  {width, phase, shade, lean},
                                  {twist, fold, hue, seed}});
    }

    return blades;
}
} // namespace Cows
