#pragma once

#include "Props/Scenery.h"

namespace Cows
{
constexpr auto fenceHeight = 1.4f;

// Runs along x; too tall to step over, low enough to jump.
void addFence(Scenery& scenery, Maths::Vec2 at, float length);
} // namespace Cows
