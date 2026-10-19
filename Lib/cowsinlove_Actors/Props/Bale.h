#pragma once

#include "Props/Mover.h"
#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
constexpr auto baleRadius = 0.8f;

constexpr auto rollingBaleRadius = 1.05f;

// In makeBarrel's frame; lyingDrum lays them down.
Vector<Mover::Part> baleParts();

void addBale(Scenery& scenery, std::mt19937& random, Maths::Vec2 at);

Mover makeRollingBale(Maths::Vec2 from, Maths::Vec2 to, float period, float phase);
} // namespace Cows
