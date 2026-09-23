#include "Grass.h"

#include <cmath>
#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto bladeCount = 14000;
constexpr auto bladeSegments = 4;
} // namespace

Blade makeBlade()
{
    auto blade = Blade {};

    for (auto segment = 0; segment < bladeSegments; ++segment)
    {
        auto height = (float) segment / (float) bladeSegments;
        blade.vertices.add({{-1.f, height}});
        blade.vertices.add({{1.f, height}});
    }

    blade.vertices.add({{0.f, 1.f}});

    for (auto segment = 0; segment + 1 < bladeSegments; ++segment)
    {
        auto a = (std::uint16_t) (segment * 2);
        blade.indices.add({a,
                           (std::uint16_t) (a + 1),
                           (std::uint16_t) (a + 3),
                           a,
                           (std::uint16_t) (a + 3),
                           (std::uint16_t) (a + 2)});
    }

    auto last = (std::uint16_t) ((bladeSegments - 1) * 2);
    blade.indices.add(
        {last, (std::uint16_t) (last + 1), (std::uint16_t) (bladeSegments * 2)});

    return blade;
}

Vector<BladeInstance> makeMeadow()
{
    auto blades = Vector<BladeInstance> {};
    blades.reserve(bladeCount);

    auto random = std::mt19937 {1994u};
    auto unit = std::uniform_real_distribution<float> {0.f, 1.f};

    while (blades.size() < bladeCount)
    {
        auto x = meadowTile * unit(random);
        auto z = meadowTile * unit(random);

        auto clump = 0.5f + 0.5f * std::sin(x * 1.3f + std::sin(z * 0.9f) * 2.f);
        auto height = 0.2f + 0.2f * unit(random) + 0.14f * clump;
        auto width = 0.018f + 0.012f * unit(random);

        auto facing = twoPi * unit(random);
        auto phase = twoPi * unit(random);
        auto shade = unit(random);
        auto lean = unit(random) - 0.5f;

        blades.add(
            BladeInstance {{x, z, facing, height}, {width, phase, shade, lean}});
    }

    return blades;
}
} // namespace Cows
