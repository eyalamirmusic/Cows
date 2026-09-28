#pragma once

#include "Level.h"

#include <cstdint>
#include <map>
#include <utility>

namespace Cows
{
// A point on the unit blade: x across it (-1 to 1), y up it (0 to 1).
struct BladeVertex final
{
    Maths::Vec2 shape;
};

// placement is (x, z, facing, height); look is (width, sway phase, shade,
// lean); form is (twist, fold, hue, seed).
struct BladeInstance final
{
    Maths::Vec4 placement;
    Maths::Vec4 look;
    Maths::Vec4 form;
};

struct Blade final
{
    Vector<BladeVertex> vertices;
    Vector<std::uint16_t> indices;
};

Blade makeBlade();

constexpr auto meadowTile = 24.f;
constexpr auto bladeCount = 14000;

// One square tile of the meadow, meadowTile on a side from the origin, laid
// down again and again round the player.
Vector<BladeInstance> makeGrassTile();

// `tile` laid at `corner` without the blades that would stand over a gap or
// on its rim.
Vector<BladeInstance> cutTile(const Vector<BladeInstance>& tile,
                              Maths::Vec2 corner,
                              const Vector<Gap>& gaps);

// The meadow tile laid over a level: tiles that cross a gap are cut, once,
// and kept; the rest are the tile itself.
struct GrassField final
{
    void layOver(const Level& level);

    const Vector<BladeInstance>& tileAt(Maths::Vec2 corner);

    Vector<BladeInstance> tile = makeGrassTile();
    Vector<Gap> gaps;
    std::map<std::pair<int, int>, Vector<BladeInstance>> cutTiles;
};
} // namespace Cows
