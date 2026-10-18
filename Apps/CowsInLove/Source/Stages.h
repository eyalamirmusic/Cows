#pragma once

#include "Levels/LevelTemplate.h"

#include <cstdint>

namespace Cows
{
// A stage is a position in the ordered list of level templates; a level is one
// generated from a template and a seed. After the last stage comes the first
// again (nothing is saved yet).
struct Stages final
{
    // Starts on `stage`, wrapped into range.
    explicit Stages(int stage = 0);

    void advance();
    LevelMaker level() const;

    Vector<LevelTemplate> templates;
    int current = 0;
};

// A seed from the clock, different each round.
std::uint32_t clockSeed();
} // namespace Cows
