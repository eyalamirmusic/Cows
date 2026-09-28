#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <functional>

namespace Cows
{
// How high feet can step up onto a surface without jumping.
constexpr auto stepUp = 0.55f;

// The props a level is laid out from, what the player collides with, and
// where the other cow hides among them.
struct Level final : Scenery
{
    enum class Hiding
    {
        Roof,
        Stack,
        Grove,
        Hedge,
        Meadow
    };

    // Moves a circle of `radius` with its feet at `feet` out of everything too
    // tall to step onto, so the player slides along whatever it walks into.
    Maths::Vec2 pushedOut(Maths::Vec2 point, float radius, float feet) const;

    // The highest surface under `point` that feet at `feet` can stand on.
    float floorAt(Maths::Vec2 point, float feet) const;

    bool isFree(Maths::Vec2 point, float margin) const;

    Maths::Vec3 hideout;
    Hiding hiding = Hiding::Roof;
};

// Builds a level from a seed: the same seed always gives the same level.
using LevelMaker = std::function<Level(std::uint32_t)>;
} // namespace Cows
