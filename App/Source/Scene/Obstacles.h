#pragma once

#include "Instances.h"

#include <cstdint>

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

// The meadow's clutter: groves of trees, hedgerows with gaps in them, hay
// bales, rocks, crates and barns with ramps and platforms up to their roofs,
// laid out from a seed. The other cow waits on one of the roofs.
struct Obstacles final
{
    explicit Obstacles(std::uint32_t seed = 1u);

    // Moves a circle of `radius` with its feet at `feet` out of everything too
    // tall to step onto, so the player slides along whatever it walks into.
    Maths::Vec2 pushedOut(Maths::Vec2 point, float radius, float feet) const;

    // The highest surface under `point` that feet at `feet` can stand on.
    float floorAt(Maths::Vec2 point, float feet) const;

    bool isFree(Maths::Vec2 point, float margin) const;

    SurfaceBatch batch;
    Vector<Collider> colliders;
    Vector<Block> blocks;
    Maths::Vec3 hideout;
};
} // namespace Cows
