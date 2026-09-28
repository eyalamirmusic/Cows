#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
void addRock(Scenery& scenery, std::mt19937& random, Maths::Vec2 at);
} // namespace Cows
