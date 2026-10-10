#pragma once

#include "Level.h"

#include <cstdint>
#include <limits>
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

// A blade of `segments` rows up to its tip.
Blade makeBlade(int segments = 5);

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

// How much grass is drawn: the tiles round the focus (tilesAround each way),
// the share of a tile's blades drawn up close, and the share drawn at a
// distance - full from the eye out to thinFrom, easing down to farShare at
// thinTo and beyond. A tile's blades are in random order, so a share of them
// is the same meadow, thinner.
struct GrassDensity final
{
    int tilesAround = 2;
    float nearShare = 1.f;
    float farShare = 1.f;
    float thinFrom = std::numeric_limits<float>::max();
    float thinTo = std::numeric_limits<float>::max();
};

// The tallest a blade stands and the furthest it leans, with room to spare:
// what a tile's box has to hold for culling never to clip a blade.
constexpr auto bladeReach = 1.f;

// One tile to draw: where its corner lies, and the share of its blades.
struct GrassDraw final
{
    Maths::Vec2 corner;
    float share = 1.f;
};

// The tiles round `focus` that can be seen through `viewProjection`, nearest
// the eye first, each with the share of blades `density` gives its distance.
Vector<GrassDraw> planGrass(Maths::Vec2 focus,
                            const Maths::Mat4& viewProjection,
                            const Maths::Vec3& eye,
                            const GrassDensity& density);

// How many of `available` blades a draw of `share` keeps, never fewer than one
// while there are any.
int bladesToDraw(int available, float share);

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
