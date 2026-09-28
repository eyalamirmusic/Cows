#pragma once

#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
constexpr auto baleRadius = 0.8f;

void addBale(Scenery& scenery, std::mt19937& random, Maths::Vec2 at);
} // namespace Cows
