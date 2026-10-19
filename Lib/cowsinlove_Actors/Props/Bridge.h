#pragma once

#include "Props/Scenery.h"

namespace Cows
{
// Runs along z, its deck flush with the ground.
void addBridge(Scenery& scenery, Maths::Vec2 center, Maths::Vec2 half);

// Runs along x with no collider: to look at, not to walk on.
void addBridgeBeam(Scenery& scenery, Maths::Vec2 center, float length);
} // namespace Cows
