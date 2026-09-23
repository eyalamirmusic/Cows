#include "Obstacles.h"
#include "Palette.h"

#include <algorithm>
#include <cmath>
#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto arenaReach = 86.f;
constexpr auto clearing = 9.f;
constexpr auto groveCount = 9;
constexpr auto hedgeCount = 16;
constexpr auto baleClusterCount = 7;
constexpr auto rockCount = 28;
constexpr auto barnCount = 5;
constexpr auto crateCount = 12;
constexpr auto stepUp = 0.55f;
constexpr auto roofHeight = 7.f;
constexpr auto barnHalf = 4.f;
constexpr auto barnSpacing = 26.f;
constexpr auto nearestHideout = 50.f;
constexpr auto furthestHideout = 75.f;
constexpr auto structureMargin = 2.5f;

constexpr std::uint32_t leafColors[] = {0x2f8f3a, 0x3a9a40, 0x2c7a30, 0x4aa844};
constexpr std::uint32_t hedgeColors[] = {0x236b28, 0x2c7a30, 0x307f2c};
constexpr std::uint32_t barkColor = 0x6b4a2f;
constexpr std::uint32_t strawColor = 0xe3bf5a;
constexpr std::uint32_t rockColors[] = {0x8a8f8a, 0x9c9a92, 0x7b807e};
constexpr std::uint32_t barnColor = 0xb5473a;
constexpr std::uint32_t roofColor = 0x6e3b2e;
constexpr std::uint32_t woodColor = 0xa9824f;
constexpr std::uint32_t rampColor = 0x9b7a4c;
constexpr std::uint32_t crateColor = 0xb08850;

struct Layout final
{
    explicit Layout(std::uint32_t seed)
        : random(seed)
    {
    }

    float unit() { return std::uniform_real_distribution<float> {0.f, 1.f}(random); }

    float between(float from, float to) { return from + (to - from) * unit(); }

    Vec2 spot()
    {
        while (true)
        {
            auto point = Vec2 {between(-arenaReach, arenaReach),
                               between(-arenaReach, arenaReach)};

            if (length(point) > clearing + 4.f)
                return point;
        }
    }

    template <std::size_t Count>
    std::uint32_t pick(const std::uint32_t (&colors)[Count])
    {
        return colors[(std::size_t) (unit() * (float) Count) % Count];
    }

    std::mt19937 random;
};

Material matte(std::uint32_t hex, float softness = 0.f, float gloss = 0.05f)
{
    auto material = Material {Palette::linear(hex)};
    material.softness = softness;
    material.gloss = gloss;
    return material;
}

bool inClearing(Vec2 point)
{
    return length(point) < clearing;
}

void addTree(Obstacles& obstacles, Layout& layout, Vec2 at)
{
    if (inClearing(at) || !obstacles.isFree(at, structureMargin))
        return;

    auto height = layout.between(2.2f, 3.2f);
    auto crown = layout.between(1.5f, 2.3f);
    auto leaves = matte(layout.pick(leafColors), 0.5f);
    auto ground = Vec3 {at.x, 0.f, at.y};

    obstacles.batch.add(Shape::Capsule,
                        makeInstance(Mat4::translation(ground)
                                         * Mat4::scale({2.4f, height + 0.6f, 2.4f}),
                                     matte(barkColor)));

    auto top = ground + Vec3 {0.f, height + crown * 0.55f, 0.f};
    obstacles.batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation(top)
                         * Mat4::scale({crown, crown * 0.85f, crown}),
                     leaves));

    if (layout.unit() < 0.6f)
    {
        auto angle = layout.between(0.f, twoPi);
        auto side = crown * 0.6f;
        auto offset =
            Vec3 {std::cos(angle) * side, -crown * 0.2f, std::sin(angle) * side};
        auto small = crown * 0.7f;
        obstacles.batch.add(
            Shape::Sphere,
            makeInstance(Mat4::translation(top + offset)
                             * Mat4::scale({small, small * 0.85f, small}),
                         matte(layout.pick(leafColors), 0.5f)));
    }

    obstacles.colliders.add({at, 0.55f});
}

void addGrove(Obstacles& obstacles, Layout& layout)
{
    auto center = layout.spot();
    auto count = 5 + (int) (layout.unit() * 6.f);

    for (auto tree = 0; tree < count; ++tree)
    {
        auto angle = layout.between(0.f, twoPi);
        auto reach = layout.between(2.f, 11.f);
        addTree(obstacles,
                layout,
                center + Vec2 {std::cos(angle) * reach, std::sin(angle) * reach});
    }
}

void addHedgePiece(Obstacles& obstacles,
                   Layout& layout,
                   Vec2 at,
                   float heading,
                   std::uint32_t color)
{
    auto height = layout.between(2.6f, 3.1f);
    auto depth = layout.between(1.5f, 1.9f);
    auto length = 2.8f;
    auto ground = Vec3 {at.x, height * 0.5f, at.y};

    obstacles.batch.add(
        Shape::Barrel,
        makeInstance(Mat4::translation(ground) * Mat4::rotationY(heading)
                         * Mat4::rotationZ(-halfPi)
                         * Mat4::scale({height, length, depth})
                         * Mat4::translation({0.f, -0.5f, 0.f}),
                     matte(color, 0.5f)));

    auto tuft = layout.between(0.6f, 0.9f);
    obstacles.batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation(ground + Vec3 {0.f, height * 0.42f, 0.f})
                         * Mat4::scale({tuft * 1.3f, tuft, tuft}),
                     matte(color, 0.5f)));

    obstacles.colliders.add({at, depth * 0.5f + 0.15f});
}

void addHedgerow(Obstacles& obstacles, Layout& layout)
{
    auto start = layout.spot();
    auto heading = layout.unit() < 0.7f
                       ? halfPi * (float) (int) (layout.unit() * 4.f)
                             + layout.between(-0.15f, 0.15f)
                       : layout.between(0.f, twoPi);
    auto direction = Vec2 {std::cos(heading), -std::sin(heading)};
    auto pieces = 6 + (int) (layout.unit() * 10.f);
    auto gap = 2 + (int) (layout.unit() * (float) (pieces - 4));
    auto color = layout.pick(hedgeColors);

    for (auto piece = 0; piece < pieces; ++piece)
    {
        if (piece == gap || piece == gap + 1)
            continue;

        auto at = start + direction * (2.2f * (float) piece);

        if (inClearing(at) || !obstacles.isFree(at, structureMargin)
            || std::abs(at.x) > arenaReach + 4.f
            || std::abs(at.y) > arenaReach + 4.f)
            continue;

        addHedgePiece(obstacles, layout, at, heading, color);
    }
}

void addBales(Obstacles& obstacles, Layout& layout)
{
    auto center = layout.spot();
    auto count = 3 + (int) (layout.unit() * 4.f);
    auto straw = matte(strawColor, 0.2f, 0.1f);

    for (auto bale = 0; bale < count; ++bale)
    {
        auto at =
            center + Vec2 {layout.between(-4.f, 4.f), layout.between(-4.f, 4.f)};

        if (inClearing(at) || !obstacles.isFree(at, structureMargin))
            continue;

        auto heading = layout.between(0.f, twoPi);
        auto radius = 0.8f;
        obstacles.batch.add(
            Shape::Barrel,
            makeInstance(Mat4::translation({at.x, radius, at.y})
                             * Mat4::rotationY(heading) * Mat4::rotationX(halfPi)
                             * Mat4::scale({radius * 2.f, 1.7f, radius * 2.f})
                             * Mat4::translation({0.f, -0.5f, 0.f}),
                         straw));

        obstacles.colliders.add({at, 1.05f, radius * 2.f});
    }
}

void addRock(Obstacles& obstacles, Layout& layout)
{
    auto at = layout.spot();

    if (!obstacles.isFree(at, structureMargin))
        return;

    auto size = layout.between(0.5f, 1.4f);
    auto rock = matte(layout.pick(rockColors), 0.1f, 0.15f);

    obstacles.batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation({at.x, size * 0.15f, at.y})
                         * Mat4::rotationY(layout.between(0.f, pi))
                         * Mat4::scale({size * 1.3f, size * 0.6f, size}),
                     rock));

    obstacles.colliders.add({at, size * 1.05f, size * 0.75f});
}
Vec2 turned(Vec2 offset, int quarterTurns)
{
    for (auto turn = 0; turn < quarterTurns; ++turn)
        offset = {-offset.y, offset.x};

    return offset;
}

Vec2 absolute(Vec2 vector)
{
    return {std::abs(vector.x), std::abs(vector.y)};
}

void addBlock(Obstacles& obstacles, const Block& block, const Material& material)
{
    obstacles.blocks.add(block);

    auto ground = Vec3 {block.center.x, 0.f, block.center.y};

    if (lengthSquared(block.rise) == 0.f)
    {
        obstacles.batch.add(Shape::Box,
                            makeInstance(Mat4::translation(ground)
                                             * Mat4::scale({block.half.x * 2.f,
                                                            block.top,
                                                            block.half.y * 2.f}),
                                         material));
        return;
    }

    auto along = dot(absolute(block.rise), block.half);
    auto across = block.half.x + block.half.y - along;
    auto heading = std::atan2(-block.rise.y, block.rise.x);

    obstacles.batch.add(
        Shape::Wedge,
        makeInstance(Mat4::translation(ground) * Mat4::rotationY(heading)
                         * Mat4::scale({along * 2.f, block.top, across * 2.f}),
                     material));
}

// A barn with a flat roof, and a ramp, then platforms a jump apart, climbing
// its side to the roof.
void addBarn(Obstacles& obstacles, Vec2 center, int quarterTurns)
{
    auto piece = [&](Vec2 offset, Vec2 half, float top, Vec2 rise, std::uint32_t hex)
    {
        auto block = Block {center + turned(offset, quarterTurns),
                            absolute(turned(half, quarterTurns)),
                            top,
                            turned(rise, quarterTurns)};
        addBlock(obstacles, block, matte(hex, 0.1f, 0.1f));
    };

    piece({0.f, 0.f}, {barnHalf, barnHalf}, roofHeight, {}, barnColor);
    piece({-8.5f, 5.5f}, {3.5f, 1.5f}, 2.f, {1.f, 0.f}, rampColor);
    piece({-3.5f, 5.5f}, {1.5f, 1.5f}, 2.f, {}, woodColor);
    piece({0.5f, 5.5f}, {1.5f, 1.5f}, 3.7f, {}, woodColor);
    piece({4.5f, 5.5f}, {1.5f, 1.5f}, 5.4f, {}, woodColor);

    auto roof = Vec3 {center.x, roofHeight - 0.25f, center.y};
    obstacles.batch.add(
        Shape::Box,
        makeInstance(Mat4::translation(roof)
                         * Mat4::scale({barnHalf * 2.3f, 0.35f, barnHalf * 2.3f}),
                     matte(roofColor, 0.1f, 0.1f)));
}

Vec2 hideoutSpot(Layout& layout)
{
    while (true)
    {
        auto angle = layout.between(0.f, twoPi);
        auto reach = layout.between(nearestHideout, furthestHideout);
        auto at = Vec2 {std::cos(angle) * reach, std::sin(angle) * reach};

        if (std::abs(at.x) < arenaReach - 14.f && std::abs(at.y) < arenaReach - 14.f)
            return at;
    }
}

void addBarns(Obstacles& obstacles, Layout& layout)
{
    auto spots = Vector<Vec2> {};
    spots.add(hideoutSpot(layout));

    for (auto attempt = 0; attempt < 400 && spots.size() < barnCount; ++attempt)
    {
        auto at = Vec2 {layout.between(-arenaReach + 14.f, arenaReach - 14.f),
                        layout.between(-arenaReach + 14.f, arenaReach - 14.f)};
        auto roomy = length(at) > 25.f
                     && std::all_of(spots.begin(),
                                    spots.end(),
                                    [&](Vec2 other)
                                    { return distance(at, other) > barnSpacing; });

        if (roomy)
            spots.add(at);
    }

    obstacles.hideout = {spots[0].x, roofHeight, spots[0].y};

    for (const auto& spot: spots)
        addBarn(obstacles, spot, (int) (layout.unit() * 4.f) % 4);
}

void addCrates(Obstacles& obstacles, Layout& layout)
{
    auto wood = matte(crateColor, 0.1f, 0.1f);

    for (auto crate = 0; crate < crateCount; ++crate)
    {
        auto at = layout.spot();

        if (!obstacles.isFree(at, structureMargin + 1.f))
            continue;

        addBlock(obstacles, {at, {0.9f, 0.9f}, 1.5f, {}}, wood);

        if (layout.unit() < 0.6f)
            addBlock(
                obstacles, {at + Vec2 {1.8f, 0.f}, {0.9f, 0.9f}, 3.f, {}}, wood);
    }
}
} // namespace

float Block::heightAt(Vec2 point) const
{
    if (lengthSquared(rise) == 0.f)
        return top;

    auto along = dot(absolute(rise), half);
    auto t = (dot(point - center, rise) / along + 1.f) * 0.5f;
    return top * std::clamp(t, 0.f, 1.f);
}

Vec2 Block::closestTo(Vec2 point) const
{
    return {std::clamp(point.x, center.x - half.x, center.x + half.x),
            std::clamp(point.y, center.y - half.y, center.y + half.y)};
}

bool Block::contains(Vec2 point, float margin) const
{
    return std::abs(point.x - center.x) <= half.x + margin
           && std::abs(point.y - center.y) <= half.y + margin;
}

Obstacles::Obstacles(std::uint32_t seed)
{
    auto layout = Layout {seed * 2654435761u + 17u};

    addBarns(*this, layout);
    addCrates(*this, layout);

    for (auto grove = 0; grove < groveCount; ++grove)
        addGrove(*this, layout);

    for (auto hedge = 0; hedge < hedgeCount; ++hedge)
        addHedgerow(*this, layout);

    for (auto cluster = 0; cluster < baleClusterCount; ++cluster)
        addBales(*this, layout);

    for (auto rock = 0; rock < rockCount; ++rock)
        addRock(*this, layout);
}

Vec2 Obstacles::pushedOut(Vec2 point, float radius, float feet) const
{
    for (auto pass = 0; pass < 2; ++pass)
    {
        for (const auto& collider: colliders)
        {
            if (collider.top <= feet + stepUp)
                continue;

            auto offset = point - collider.center;
            auto reach = collider.radius + radius;
            auto gap = length(offset);

            if (gap >= reach)
                continue;

            auto away = gap > 1e-4f ? offset / gap : Vec2 {1.f, 0.f};
            point = collider.center + away * reach;
        }

        for (const auto& block: blocks)
        {
            auto closest = block.closestTo(point);

            if (block.heightAt(closest) <= feet + stepUp)
                continue;

            auto offset = point - closest;
            auto gap = length(offset);

            if (gap >= radius)
                continue;

            if (gap > 1e-4f)
            {
                point = closest + offset / gap * radius;
                continue;
            }

            auto inside = point - block.center;
            auto depth = block.half - absolute(inside);

            if (depth.x < depth.y)
                point.x =
                    block.center.x + std::copysign(block.half.x + radius, inside.x);
            else
                point.y =
                    block.center.y + std::copysign(block.half.y + radius, inside.y);
        }
    }

    return point;
}

float Obstacles::floorAt(Vec2 point, float feet) const
{
    auto floor = 0.f;

    for (const auto& collider: colliders)
        if (collider.top <= feet + stepUp
            && distance(point, collider.center) < collider.radius)
            floor = std::max(floor, collider.top);

    for (const auto& block: blocks)
    {
        if (!block.contains(point))
            continue;

        auto height = block.heightAt(point);

        if (height <= feet + stepUp)
            floor = std::max(floor, height);
    }

    return floor;
}

bool Obstacles::isFree(Vec2 point, float margin) const
{
    return std::none_of(blocks.begin(),
                        blocks.end(),
                        [&](const Block& block)
                        { return block.contains(point, margin); });
}
} // namespace Cows
