#pragma once

#include "Levels/LevelTemplate.h"

#include <cstdint>

namespace Cows
{
// What Cows in Love plays: its levels in order, each a fresh seed each round,
// back to the first after the last (nothing is saved yet).
struct Stages final
{
    Stages();

    // COWS_SEED when set, else the clock.
    std::uint32_t firstSeed() const;
    std::uint32_t nextSeed() const;

    void advance();
    LevelMaker level() const;

    Vector<LevelTemplate> templates;
    // COWS_STAGE when set, else the first.
    int current = 0;
};
} // namespace Cows
