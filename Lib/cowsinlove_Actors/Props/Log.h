#pragma once

#include "Props/Scenery.h"

namespace Cows
{
constexpr auto logRadius = 0.6f;

// A felled log lying along x, centred on `at`: too tall to step over, low
// enough to jump.
void addLog(Scenery& scenery, Maths::Vec2 at, float length);
} // namespace Cows
