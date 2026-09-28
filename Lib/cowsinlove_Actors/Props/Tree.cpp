#include "Props/Tree.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t leafColors[] = {0x2f8f3a, 0x3a9a40, 0x2c7a30, 0x4aa844};
constexpr std::uint32_t barkColor = 0x6b4a2f;
} // namespace

void addTree(Scenery& scenery, std::mt19937& random, Vec2 at)
{
    auto height = randomBetween(random, 2.2f, 3.2f);
    auto crown = randomBetween(random, 1.5f, 2.3f);
    auto leaves = matte(randomPick(random, leafColors), 0.5f);
    auto ground = Vec3 {at.x, 0.f, at.y};

    scenery.batch.add(Shape::Capsule,
                      makeInstance(Mat4::translation(ground)
                                       * Mat4::scale({2.4f, height + 0.6f, 2.4f}),
                                   matte(barkColor)));

    auto top = ground + Vec3 {0.f, height + crown * 0.55f, 0.f};
    scenery.batch.add(Shape::Sphere,
                      makeInstance(Mat4::translation(top)
                                       * Mat4::scale({crown, crown * 0.85f, crown}),
                                   leaves));

    if (randomUnit(random) < 0.6f)
    {
        auto angle = randomBetween(random, 0.f, twoPi);
        auto side = crown * 0.6f;
        auto offset =
            Vec3 {std::cos(angle) * side, -crown * 0.2f, std::sin(angle) * side};
        auto small = crown * 0.7f;
        scenery.batch.add(
            Shape::Sphere,
            makeInstance(Mat4::translation(top + offset)
                             * Mat4::scale({small, small * 0.85f, small}),
                         matte(randomPick(random, leafColors), 0.5f)));
    }

    scenery.colliders.add({at, 0.55f});
}
} // namespace Cows
