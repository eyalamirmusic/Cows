#include "Props/Log.h"
#include "Props/Props.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t barkColor = 0x6b4a2e;
constexpr std::uint32_t ringColor = 0xc9a26b;
} // namespace

void addLog(Scenery& scenery, Vec2 at, float length)
{
    auto diameter = logRadius * 2.f;
    scenery.blocks.add({at, {length * 0.5f, logRadius}, diameter, {}});

    scenery.batch.add(Shape::Barrel,
                      makeInstance(Mat4::translation({at.x, logRadius, at.y})
                                       * Mat4::rotationZ(-halfPi)
                                       * Mat4::scale({diameter, length, diameter})
                                       * Mat4::translation({0.f, -0.5f, 0.f}),
                                   matte(barkColor, 0.1f, 0.05f)));

    for (auto end: {-0.5f, 0.5f})
        scenery.batch.add(
            Shape::Sphere,
            makeInstance(
                Mat4::translation({at.x + end * (length - 0.1f), logRadius, at.y})
                    * Mat4::scale({0.08f, logRadius * 0.8f, logRadius * 0.8f}),
                matte(ringColor, 0.1f, 0.05f)));
}
} // namespace Cows
