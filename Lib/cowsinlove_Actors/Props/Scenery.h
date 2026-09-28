#pragma once

#include "Props/Collision.h"
#include "Render/Instances.h"

namespace Cows
{
// What props are added to: their instances, and the shapes they collide with.
struct Scenery
{
    SurfaceBatch batch;
    Vector<Collider> colliders;
    Vector<Block> blocks;
};
} // namespace Cows
