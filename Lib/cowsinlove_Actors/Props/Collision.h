#pragma once

#include "Render/Common.h"

namespace Cows
{
// A round obstacle, standing `top` high.
struct Collider final
{
    Maths::Vec2 center;
    float radius = 1.f;
    float top = 100.f;
};

// A solid box standing on the ground, `half` its size on each axis; a ramp
// when `rise` is set, its top climbing from the ground to `top` along it.
struct Block final
{
    float heightAt(Maths::Vec2 point) const;
    Maths::Vec2 closestTo(Maths::Vec2 point) const;
    bool contains(Maths::Vec2 point, float margin = 0.f) const;

    Maths::Vec2 center;
    Maths::Vec2 half;
    float top = 1.f;
    Maths::Vec2 rise;
};

Maths::Vec2 absolute(Maths::Vec2 vector);
} // namespace Cows
