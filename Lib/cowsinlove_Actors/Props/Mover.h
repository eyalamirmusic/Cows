#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// A barrel lying on its side that rolls back and forth between `from` and
// `to` on the ground at `lift`, once each `period` seconds, `phase` of the way
// through at zero. Its collider follows it.
struct Mover final
{
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
    Material material;
};
} // namespace Cows
