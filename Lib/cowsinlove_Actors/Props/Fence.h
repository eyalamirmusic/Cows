#pragma once

#include "Props/Scenery.h"

namespace Cows
{
constexpr auto fenceHeight = 1.4f;

// A post-and-rail fence running along x, centred on `at`: too tall to step
// over, low enough to jump.
void addFence(Scenery& scenery, Maths::Vec2 at, float length);
} // namespace Cows
