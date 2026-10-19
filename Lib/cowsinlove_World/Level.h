#pragma once

#include "Levels/Region.h"
#include "Props/Mover.h"
#include "Props/Scenery.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace Cows
{
constexpr auto maxStepUpHeight = 0.55f;

float headingToward(Maths::Vec2 direction);

struct Gap final
{
    bool contains(Maths::Vec2 point) const;

    Maths::Vec2 center;
    Maths::Vec2 half;
    float depth = -30.f;
};

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

    Maths::Vec2 pushedOut(Maths::Vec2 point, float radius, float feet) const;

    Maths::Vec2 keptInGap(Maths::Vec2 point, float radius, float feet) const;

    float floorAt(Maths::Vec2 point, float feet) const;

    // Infinity when nothing floats overhead.
    float ceilingAt(Maths::Vec2 point, float feet) const;

    bool isFree(Maths::Vec2 point, float margin) const;

    // Passes through blocks that `from` is already inside.
    Maths::Vec3 clearReach(Maths::Vec3 from, Maths::Vec3 to, float margin) const;

    float groundAt(Maths::Vec2 point) const;
    bool overGap(Maths::Vec2 point) const;

    bool onMover(Maths::Vec2 point, float feet) const;

    void update(float seconds);

    Maths::Vec3 hideout;
    Hiding hiding = Hiding::Roof;

    // Unset: the ending faces from the player to her.
    std::optional<float> hideoutHeading;

    Maths::Vec3 start;
    float startHeading = 0.f;

    Vector<Gap> gaps;
    Vector<Mover> movers;
    SurfaceBatch moving;

    Region criticalPath;

    // 0 means the player never respawns from a fall.
    float killDepth = 0.f;
};

using LevelMaker = std::function<Level(std::uint32_t)>;
} // namespace Cows
