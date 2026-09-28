#include "Props/Bridge.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t plankColor = 0x9b7a4c;
constexpr std::uint32_t beamColor = 0x6e5236;
constexpr auto deckThickness = 0.6f;
constexpr auto deckLift = 0.04f;
constexpr auto plankLength = 1.2f;
} // namespace

void addBridge(Scenery& scenery, Vec2 center, Vec2 half)
{
    scenery.blocks.add({center, half, 0.f, {}});

    auto planks = std::max(1, (int) std::round(half.y * 2.f / plankLength));
    auto step = half.y * 2.f / (float) planks;

    for (auto plank = 0; plank < planks; ++plank)
    {
        auto z = center.y - half.y + step * ((float) plank + 0.5f);
        auto shade = plank % 2 == 0 ? 1.f : 0.88f;
        auto material = matte(plankColor, 0.1f, 0.1f);
        material.color = material.color * shade;

        scenery.batch.add(
            Shape::Box,
            makeInstance(
                Mat4::translation({center.x, deckLift - deckThickness, z})
                    * Mat4::scale({half.x * 2.f, deckThickness, step * 0.94f}),
                material));
    }

    for (auto side: {-1.f, 1.f})
        scenery.batch.add(
            Shape::Box,
            makeInstance(Mat4::translation(
                             {center.x + side * (half.x - 0.15f), -1.4f, center.y})
                             * Mat4::scale({0.3f, 1.2f, half.y * 2.f}),
                         matte(beamColor, 0.1f, 0.1f)));
}

void addBridgeBeam(Scenery& scenery, Vec2 center, float length)
{
    scenery.batch.add(Shape::Box,
                      makeInstance(Mat4::translation({center.x, -0.5f, center.y})
                                       * Mat4::scale({length, 0.48f, 1.f}),
                                   matte(beamColor, 0.1f, 0.1f)));
}
} // namespace Cows
