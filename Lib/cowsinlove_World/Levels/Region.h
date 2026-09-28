#pragma once

#include "Render/Common.h"

namespace Cows
{
constexpr auto arenaSize = 200.f;

// An axis-aligned rectangle of the ground, x and z.
struct Region final
{
    static Region arena();

    bool contains(Maths::Vec2 point, float margin = 0.f) const;
    bool holds(Maths::Vec2 point) const;
    Region inset(float by) const;
    bool isEmpty() const;
    Maths::Vec2 center() const;
    Maths::Vec2 size() const;
    float area() const;

    Maths::Vec2 min;
    Maths::Vec2 max;
};
} // namespace Cows
