#include "Props/Rock.h"
#include "Props/Props.h"

#include <array>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto rockColors =
    std::to_array<std::uint32_t>({0x8a8f8a, 0x9c9a92, 0x7b807e});
} // namespace

void addRock(Scenery& scenery, std::mt19937& random, Vec2 at)
{
    auto size = randomBetween(random, 0.5f, 1.4f);
    auto rock = matte(randomPick(random, rockColors), 0.1f, 0.15f);

    scenery.batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation({at.x, size * 0.15f, at.y})
                         * Mat4::rotationY(randomBetween(random, 0.f, pi))
                         * Mat4::scale({size * 1.3f, size * 0.6f, size}),
                     rock));

    scenery.colliders.add({at, size * 1.05f, size * 0.75f});
}
} // namespace Cows
