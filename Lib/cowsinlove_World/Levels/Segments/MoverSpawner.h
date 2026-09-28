#pragma once

#include "Level.h"

namespace Cows
{
// Emits a rolling bale into a level, going to and fro between `from` and
// `to` once each `period` seconds, `phase` of the way through at zero.
struct MoverSpawner final
{
    void spawn(Level& level) const;

    Maths::Vec2 from;
    Maths::Vec2 to;
    float period = 6.f;
    float phase = 0.f;
};
} // namespace Cows
