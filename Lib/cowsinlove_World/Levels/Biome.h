#pragma once

#include "Levels/Layout.h"

namespace Cows
{
// Counts are for the whole arena, scaled to each region's share.
struct Biome final
{
    int barns = 5;
    int crates = 12;
    int groves = 9;
    int hedgerows = 16;
    int baleClusters = 7;
    int rocks = 28;
};

Biome meadowBiome();

void populate(Level& level, Layout& layout, Region region, const Biome& biome);
} // namespace Cows
