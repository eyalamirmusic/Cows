#include "Levels/Biome.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
Region southHalf()
{
    return {{-100.f, 0.f}, {100.f, 100.f}};
}
} // namespace

auto tMeadowCounts = test("Biome/meadowCounts") = []
{
    auto biome = meadowBiome();
    check(biome.barns == 5 && biome.crates == 12 && biome.groves == 9);
    check(biome.hedgerows == 16 && biome.baleClusters == 7 && biome.rocks == 28);
};

auto tShare = test("Biome/countsScaleWithTheRegion") = []
{
    auto layout = Layout {1u};
    check(layout.share(28, Region::arena()) == 28);
    check(layout.share(28, southHalf()) == 14);
    check(layout.share(9, {{-100.f, 0.f}, {100.f, 10.f}}) == 0);
};

auto tStaysInRegion = test("Biome/populateStaysInItsRegion") = []
{
    for (auto seed = 1u; seed <= 10u; ++seed)
    {
        auto level = Level {};
        auto layout = Layout {seed};
        layout.home = {0.f, 50.f};
        auto region = southHalf();
        populate(level, layout, region, meadowBiome());

        check(!level.colliders.empty());

        for (const auto& collider: level.colliders)
            check(region.contains(collider.center));

        for (const auto& block: level.blocks)
        {
            check(region.contains(block.center - block.half));
            check(region.contains(block.center + block.half));
        }
    }
};

auto tAvoidsPath = test("Biome/populateKeepsOffThePath") = []
{
    for (auto seed = 1u; seed <= 10u; ++seed)
    {
        auto level = Level {};
        auto layout = Layout {seed};
        layout.home = {0.f, 50.f};
        layout.path = {{-3.f, -100.f}, {3.f, 50.f}};
        populate(level, layout, southHalf(), meadowBiome());

        for (const auto& collider: level.colliders)
            check(!layout.path.contains(collider.center, collider.radius));

        for (const auto& block: level.blocks)
            check(block.center.x + block.half.x < -3.f
                  || block.center.x - block.half.x > 3.f
                  || block.center.y - block.half.y > 50.f);
    }
};

auto tClearing = test("Biome/populateLeavesTheClearing") = []
{
    auto level = Level {};
    auto layout = Layout {4u};
    layout.home = {10.f, 40.f};
    populate(level, layout, southHalf(), meadowBiome());

    for (const auto& collider: level.colliders)
        check(distance(collider.center, layout.home) > clearing - 1.f);
};
