#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// A rectangle of ground under a roof, where no grass grows.
struct Clearing final
{
    Maths::Vec2 center;
    Maths::Vec2 half;
};

// What props are added to: their instances, the shapes they collide with, and
// the ground they keep the grass off.
struct Scenery
{
    SurfaceBatch batch;
    Vector<Collider> colliders;
    Vector<Block> blocks;
    Vector<Clearing> clearings;
};
} // namespace Cows
