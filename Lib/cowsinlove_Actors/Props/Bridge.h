#pragma once

#include "Props/Scenery.h"

namespace Cows
{
// A plank bridge along z, its deck flush with the ground, `half` its size.
void addBridge(Scenery& scenery, Maths::Vec2 center, Maths::Vec2 half);

// A beam across a bridge along x, for a bale to roll on: to look at, not to
// walk on.
void addBridgeBeam(Scenery& scenery, Maths::Vec2 center, float length);
} // namespace Cows
