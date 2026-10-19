#pragma once

#include "Render/Common.h"

namespace Cows
{
struct Collider final
{
    Maths::Vec2 center;
    float radius = 1.f;
    float top = 100.f;
};

// Feet below `bottom` pass under it; with `rise` set it ramps up to `top`.
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
