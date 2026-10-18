#pragma once

#include "Levels/Region.h"
#include "Props/Mover.h"
#include "Props/Scenery.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace Cows
{
// How high feet can step up onto a surface without jumping.
constexpr auto stepUp = 0.55f;

// The heading that faces along `direction`, as the player's and the ending
// stage's headings are measured.
float headingToward(Maths::Vec2 direction);

// A hole in the ground, `half` its size on each axis, its floor at `depth`.
struct Gap final
{
    bool contains(Maths::Vec2 point) const;

    Maths::Vec2 center;
    Maths::Vec2 half;
    float depth = -30.f;
};

// The props a level is laid out from, what the player collides with, and
// where the other cow hides among them. The ground is solid everywhere but
// its gaps.
struct Level final : Scenery
{
    enum class Hiding
    {
        Roof,
        Stack,
        Grove,
        Hedge,
        Meadow,
        Loft
    };

    // Moves a circle of `radius` with its feet at `feet` out of everything too
    // tall to step onto, so the player slides along whatever it walks into.
    Maths::Vec2 pushedOut(Maths::Vec2 point, float radius, float feet) const;

    // Below a gap's rim its walls hold the circle in.
    Maths::Vec2 keptInGap(Maths::Vec2 point, float radius, float feet) const;

    // The highest surface under `point` that feet at `feet` can stand on.
    float floorAt(Maths::Vec2 point, float feet) const;

    // The lowest bottom of the floating blocks over feet at `feet` at `point`
    // (a loft, a roof), which a jump from there cannot rise into; infinity
    // when the sky is open.
    float ceilingAt(Maths::Vec2 point, float feet) const;

    // Whether nothing stands within `margin` of `point`, a floating block
    // included: for placing things.
    bool isFree(Maths::Vec2 point, float margin) const;

    // How far along the line from `from` to `to` is clear of blocks: the
    // point `margin` short of the first block it enters, or `to`. Blocks
    // `from` is already inside are passed through.
    Maths::Vec3 clearReach(Maths::Vec3 from, Maths::Vec3 to, float margin) const;

    // The ground's height at `point`: 0, or a gap's floor.
    float groundAt(Maths::Vec2 point) const;
    bool overGap(Maths::Vec2 point) const;

    // Whether feet at `feet` stand on a mover at `point`.
    bool onMover(Maths::Vec2 point, float feet) const;

    // Moves every mover to where it is `seconds` in, and rebuilds `moving`.
    void update(float seconds);

    Maths::Vec3 hideout;
    Hiding hiding = Hiding::Roof;

    // The way the ending's stage faces when she is found there; unset, it
    // faces from the player to her.
    std::optional<float> hideoutHeading;

    Maths::Vec3 start;
    float startHeading = 0.f;

    Vector<Gap> gaps;
    Vector<Mover> movers;
    SurfaceBatch moving;

    // The strip from the start to the last segment that nothing is placed on
    // but the level's own obstacles; empty when there is none.
    Region criticalPath;

    // Falling below this puts the player back on firm ground; 0 never does.
    float killDepth = 0.f;
};

// Builds a level from a seed: the same seed always gives the same level.
using LevelMaker = std::function<Level(std::uint32_t)>;
} // namespace Cows
