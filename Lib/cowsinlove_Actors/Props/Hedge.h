#pragma once

#include "Props/Scenery.h"

#include <array>
#include <cstdint>
#include <random>

namespace Cows
{
inline constexpr auto hedgeColors =
    std::to_array<std::uint32_t>({0x236b28, 0x2c7a30, 0x307f2c});

struct HedgePiece final
{
    Maths::Vec2 at;
    float heading = 0.f;
    std::uint32_t color = hedgeColors[0];
};

void addHedgePiece(Scenery& scenery, std::mt19937& random, const HedgePiece& piece);
} // namespace Cows
