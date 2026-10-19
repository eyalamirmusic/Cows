#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
// Returns the top of the crate, or of the stack when it stacks a second.
Maths::Vec3 addCrate(Scenery& scenery, std::mt19937& random, Maths::Vec2 at);
} // namespace Cows
