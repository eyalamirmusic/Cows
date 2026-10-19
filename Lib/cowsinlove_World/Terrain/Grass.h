#pragma once

#include "Level.h"

#include <cstdint>
#include <limits>
#include <map>
#include <utility>

namespace Cows
{
// x across the blade (-1 to 1), y up it (0 to 1).
struct BladeVertex final
{
    Maths::Vec2 shape;
};

// placement (x, z, facing, height), look (width, sway, shade, lean),
// form (twist, fold, hue, seed).
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

Blade makeBlade(int segments = 5);

constexpr auto meadowTile = 24.f;
constexpr auto bladeCount = 14000;

Vector<BladeInstance> makeGrassTile();

Vector<BladeInstance> cutTile(const Vector<BladeInstance>& tile,
                              Maths::Vec2 corner,
                              const Vector<Gap>& gaps);

// A tile's blades are in random order, so a share of them is the same meadow.
struct GrassDensity final
{
    int tilesAround = 2;
    float nearShare = 1.f;
    float farShare = 1.f;
    float thinFrom = std::numeric_limits<float>::max();
    float thinTo = std::numeric_limits<float>::max();
};

// Padding on a tile's box so culling never clips a blade.
constexpr auto bladeReach = 1.f;

struct GrassDraw final
{
    Maths::Vec2 corner;
    float share = 1.f;
};

// Nearest the eye first.
Vector<GrassDraw> planGrass(Maths::Vec2 focus,
                            const Maths::Mat4& viewProjection,
                            const Maths::Vec3& eye,
                            const GrassDensity& density);

// Never fewer than one while any are available.
int bladesToDraw(int available, float share);

struct GrassField final
{
    void layOver(const Level& level);

    const Vector<BladeInstance>& tileAt(Maths::Vec2 corner);

    Vector<BladeInstance> tile = makeGrassTile();
    Vector<Gap> gaps;
    std::map<std::pair<int, int>, Vector<BladeInstance>> cutTiles;
};
} // namespace Cows
