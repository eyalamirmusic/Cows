#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// Ground under a roof, kept free of grass.
struct Clearing final
{
    Maths::Vec2 center;
    Maths::Vec2 half;
};

struct Scenery
{
    SurfaceBatch batch;
    Vector<Collider> colliders;
    Vector<Block> blocks;
    Vector<Clearing> clearings;
};
} // namespace Cows
