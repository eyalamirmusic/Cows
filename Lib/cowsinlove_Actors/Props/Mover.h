#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// The model that lays makeBarrel's frame (along y from 0 to 1, half a unit in
// radius) on its side `radius` and `length` big, its middle over `at` on the
// ground at `lift`, its axis across `heading`, `roll` turned about it.
Maths::Mat4 lyingDrum(Maths::Vec2 at,
                      float lift,
                      float heading,
                      float roll,
                      float radius,
                      float length);

// A drum lying on its side that rolls back and forth between `from` and `to`
// on the ground at `lift`, once each `period` seconds, `phase` of the way
// through at zero, drawn as its `parts`. Its collider follows it.
struct Mover final
{
    struct Part final
    {
        Shape shape;
        Material material;
    };

    Maths::Vec2 positionAt(float seconds) const;
    Maths::Mat4 modelAt(float seconds) const;

    Collider collider;
    Maths::Vec2 from;
    Maths::Vec2 to;
    float period = 6.f;
    float phase = 0.f;
    float radius = 1.f;
    float width = 1.8f;
    float lift = 0.f;
    Vector<Part> parts;
};
} // namespace Cows
