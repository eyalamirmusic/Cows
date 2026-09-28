#pragma once

#include "Levels/Layout.h"

namespace Cows
{
// What dresses a region: how many of each prop the whole arena would hold,
// scaled to the region's share of it.
struct Biome final
{
    int barns = 5;
    int crates = 12;
    int groves = 9;
    int hedgerows = 16;
    int baleClusters = 7;
    int rocks = 28;
};

// Groves of trees, hedgerows with gaps in them, hay bales, rocks, crates and
// barns with ramps and platforms up to their roofs.
Biome meadowBiome();

// The populate pass: dresses `region` with `biome`'s props, never in the
// clearing round the start or on the critical path, and notes where she
// could hide among them.
void populate(Level& level, Layout& layout, Region region, const Biome& biome);
} // namespace Cows
