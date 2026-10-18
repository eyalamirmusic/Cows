#pragma once

#include "Props/Mover.h"
#include "Props/Scenery.h"

#include <cstdint>
#include <random>

namespace Cows
{
constexpr auto baleRadius = 0.8f;

constexpr auto rollingBaleRadius = 1.05f;

// The round bale's meshes (Resources/bale.mesh, tools/blender/bale.py) with
// their colours: straw, and darker twine round the drum. They stand in
// makeBarrel's frame; `lyingDrum` lays them down.
Vector<Mover::Part> baleParts();

void addBale(Scenery& scenery, std::mt19937& random, Maths::Vec2 at);

// A big bale rolling to and fro between `from` and `to`, too tall to step onto.
Mover makeRollingBale(Maths::Vec2 from, Maths::Vec2 to, float period, float phase);
} // namespace Cows
