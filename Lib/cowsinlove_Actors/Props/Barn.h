#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
constexpr auto barnRoofHeight = 7.f;

// A barn with a flat roof, and a ramp, then platforms a jump apart, climbing
// its side to the roof.
void addBarn(Scenery& scenery, Maths::Vec2 center, int quarterTurns);
} // namespace Cows
