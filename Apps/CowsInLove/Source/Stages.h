#pragma once

#include "Levels/Meadow.h"

#include <cstdint>

namespace Cows
{
// What Cows in Love plays: an endless run of meadows, a fresh seed each round.
struct Stages final
{
    // COWS_SEED when set, else the clock.
    std::uint32_t firstSeed() const;
    std::uint32_t nextSeed() const;

    LevelMaker level = makeMeadow;
};
} // namespace Cows
