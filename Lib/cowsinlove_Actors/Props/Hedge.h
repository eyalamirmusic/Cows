#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
inline constexpr std::uint32_t hedgeColors[] = {0x236b28, 0x2c7a30, 0x307f2c};

void addHedgePiece(Scenery& scenery,
                   std::mt19937& random,
                   Maths::Vec2 at,
                   float heading,
                   std::uint32_t color);
} // namespace Cows
