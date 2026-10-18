#include "Props/Mover.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
float travelled(const Mover& mover, float seconds)
{
    auto turn = twoPi * (seconds / mover.period + mover.phase);
    return 0.5f - 0.5f * std::cos(turn);
}
} // namespace

Mat4 lyingDrum(
    Vec2 at, float lift, float heading, float roll, float radius, float length)
{
    return Mat4::translation({at.x, lift + radius, at.y}) * Mat4::rotationY(heading)
           * Mat4::rotationZ(roll) * Mat4::rotationX(halfPi)
           * Mat4::scale({radius * 2.f, length, radius * 2.f})
           * Mat4::translation({0.f, -0.5f, 0.f});
}

Vec2 Mover::positionAt(float seconds) const
{
    return from + (to - from) * travelled(*this, seconds);
}

Mat4 Mover::modelAt(float seconds) const
{
    auto track = to - from;
    auto along = length(track) * travelled(*this, seconds);
    auto heading = std::atan2(-track.y, track.x);
    auto at = positionAt(seconds);

    return lyingDrum(at, lift, heading, -along / radius, radius, width);
}
} // namespace Cows
