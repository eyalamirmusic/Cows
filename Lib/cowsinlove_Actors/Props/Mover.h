#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// Lays makeBarrel's frame (y 0 to 1, radius 0.5) on its side across `heading`.
struct DrumPlacement final
{
    Maths::Vec2 at;
    float lift = 0.f;
    float heading = 0.f;
    float roll = 0.f;
    float radius = 0.f;
    float length = 0.f;
};

Maths::Mat4 lyingDrum(const DrumPlacement& drum);

// `period` in seconds; `phase` is the share of a period done at zero.
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
