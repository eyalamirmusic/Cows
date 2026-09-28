#pragma once

#include "Level.h"

#include <cstdint>

namespace Cows
{
// The meadow's clutter: groves of trees, hedgerows with gaps in them, hay
// bales, rocks, crates and barns with ramps and platforms up to their roofs,
// laid out from a seed. The other cow hides somewhere among them.
Level makeMeadow(std::uint32_t seed);
} // namespace Cows
