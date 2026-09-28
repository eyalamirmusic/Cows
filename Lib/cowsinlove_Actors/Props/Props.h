#pragma once

#include "Props/Scenery.h"

#include <cstddef>
#include <cstdint>
#include <random>

namespace Cows
{
float randomUnit(std::mt19937& random);
float randomBetween(std::mt19937& random, float from, float to);

template <std::size_t Count>
std::uint32_t randomPick(std::mt19937& random, const std::uint32_t (&colors)[Count])
{
    return colors[(std::size_t) (randomUnit(random) * (float) Count) % Count];
}

Material matte(std::uint32_t hex, float softness = 0.f, float gloss = 0.05f);

void addBlock(Scenery& scenery, const Block& block, const Material& material);
} // namespace Cows
