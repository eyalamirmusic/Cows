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

// A solid box, `half` its size on each axis, standing on the ground or, with
// `bottom` set, floating above it (a loft, a lintel, a roof): anything
// whose feet are below `bottom` passes under it. A ramp when `rise` is set,
// its top climbing from the ground to `top` along it.
struct Block final
{
    float heightAt(Maths::Vec2 point) const;
    Maths::Vec2 closestTo(Maths::Vec2 point) const;
    bool contains(Maths::Vec2 point, float margin = 0.f) const;

    Maths::Vec2 center;
    Maths::Vec2 half;
    float top = 1.f;
    Maths::Vec2 rise;
    float bottom = 0.f;
};

Maths::Vec2 absolute(Maths::Vec2 vector);
} // namespace Cows
