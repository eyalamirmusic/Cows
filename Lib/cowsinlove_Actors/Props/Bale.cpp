#include "Props/Bale.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t strawColor = 0xe3bf5a;
} // namespace

void addBale(Scenery& scenery, std::mt19937& random, Vec2 at)
{
    auto straw = matte(strawColor, 0.2f, 0.1f);
    auto heading = randomBetween(random, 0.f, twoPi);
    auto radius = baleRadius;
    scenery.batch.add(
        Shape::Barrel,
        makeInstance(Mat4::translation({at.x, radius, at.y})
                         * Mat4::rotationY(heading) * Mat4::rotationX(halfPi)
                         * Mat4::scale({radius * 2.f, 1.7f, radius * 2.f})
                         * Mat4::translation({0.f, -0.5f, 0.f}),
                     straw));

    scenery.colliders.add({at, 1.05f, radius * 2.f});
}
} // namespace Cows
