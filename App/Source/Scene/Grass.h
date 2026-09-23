#pragma once

#include "Common.h"

#include <cstdint>

namespace Cows
{
// A point on the unit blade: x across it (-1 to 1), y up it (0 to 1).
struct BladeVertex final
{
    Maths::Vec2 shape;
};

// placement is (x, z, facing, height); look is (width, sway phase, shade, lean).
struct BladeInstance final
{
    Maths::Vec4 placement;
    Maths::Vec4 look;
};

struct Blade final
{
    Vector<BladeVertex> vertices;
    Vector<std::uint16_t> indices;
};

Blade makeBlade();

constexpr auto meadowTile = 24.f;

// One square tile of the meadow, meadowTile on a side from the origin, laid
// down again and again round the player.
Vector<BladeInstance> makeMeadow();
} // namespace Cows
