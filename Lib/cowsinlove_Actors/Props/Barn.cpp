#include "Props/Barn.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto barnHalf = 4.f;

constexpr std::uint32_t barnColor = 0xb5473a;
constexpr std::uint32_t roofColor = 0x6e3b2e;
constexpr std::uint32_t woodColor = 0xa9824f;
constexpr std::uint32_t rampColor = 0x9b7a4c;

Vec2 turned(Vec2 offset, int quarterTurns)
{
    for (auto turn = 0; turn < quarterTurns; ++turn)
        offset = {-offset.y, offset.x};

    return offset;
}
} // namespace

void addBarn(Scenery& scenery, Vec2 center, int quarterTurns)
{
    auto piece = [&](Vec2 offset, Vec2 half, float top, Vec2 rise, std::uint32_t hex)
    {
        auto block = Block {center + turned(offset, quarterTurns),
                            absolute(turned(half, quarterTurns)),
                            top,
                            turned(rise, quarterTurns)};
        addBlock(scenery, block, matte(hex, 0.1f, 0.1f));
    };

    piece({0.f, 0.f}, {barnHalf, barnHalf}, barnRoofHeight, {}, barnColor);
    piece({-8.5f, 5.5f}, {3.5f, 1.5f}, 2.f, {1.f, 0.f}, rampColor);
    piece({-3.5f, 5.5f}, {1.5f, 1.5f}, 2.f, {}, woodColor);
    piece({0.5f, 5.5f}, {1.5f, 1.5f}, 3.7f, {}, woodColor);
    piece({4.5f, 5.5f}, {1.5f, 1.5f}, 5.4f, {}, woodColor);

    auto roof = Vec3 {center.x, barnRoofHeight - 0.25f, center.y};
    scenery.batch.add(
        Shape::Box,
        makeInstance(Mat4::translation(roof)
                         * Mat4::scale({barnHalf * 2.3f, 0.35f, barnHalf * 2.3f}),
                     matte(roofColor, 0.1f, 0.1f)));
}
} // namespace Cows
