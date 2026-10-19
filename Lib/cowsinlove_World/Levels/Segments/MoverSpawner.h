#pragma once

#include "Level.h"

namespace Cows
{
struct MoverSpawner final
{
    void spawn(Level& level) const;

    Maths::Vec2 from;
    Maths::Vec2 to;
    float period = 6.f;
    float phase = 0.f;
};
} // namespace Cows
