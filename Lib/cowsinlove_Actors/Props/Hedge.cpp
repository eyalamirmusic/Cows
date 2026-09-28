#include "Props/Hedge.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
void addHedgePiece(Scenery& scenery,
                   std::mt19937& random,
                   Vec2 at,
                   float heading,
                   std::uint32_t color)
{
    auto height = randomBetween(random, 2.6f, 3.1f);
    auto depth = randomBetween(random, 1.5f, 1.9f);
    auto length = 2.8f;
    auto ground = Vec3 {at.x, height * 0.5f, at.y};

    scenery.batch.add(
        Shape::Barrel,
        makeInstance(Mat4::translation(ground) * Mat4::rotationY(heading)
                         * Mat4::rotationZ(-halfPi)
                         * Mat4::scale({height, length, depth})
                         * Mat4::translation({0.f, -0.5f, 0.f}),
                     matte(color, 0.5f)));

    auto tuft = randomBetween(random, 0.6f, 0.9f);
    scenery.batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation(ground + Vec3 {0.f, height * 0.42f, 0.f})
                         * Mat4::scale({tuft * 1.3f, tuft, tuft}),
                     matte(color, 0.5f)));

    scenery.colliders.add({at, depth * 0.5f + 0.15f});
}
} // namespace Cows
