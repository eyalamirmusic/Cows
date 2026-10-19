#pragma once

#include "Levels/LevelTemplate.h"

#include <cstdint>

namespace Cows
{
struct Stages final
{
    explicit Stages(int stage = 0);

    void advance();
    LevelMaker level() const;

    Vector<LevelTemplate> templates;
    int current = 0;
};

std::uint32_t clockSeed();
} // namespace Cows
