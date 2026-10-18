#include "Props/Bale.h"
#include "Props/Props.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t strawColor = 0xe3bf5a;
constexpr std::uint32_t twineColor = 0x8c6a3c;
constexpr auto baleSoftness = 0.2f;
constexpr auto baleGloss = 0.1f;
constexpr auto baleLength = 1.7f;
constexpr auto baleColliderRadius = 1.05f;
} // namespace

Vector<Mover::Part> baleParts()
{
    return {{Shape::BaleStraw, matte(strawColor, baleSoftness, baleGloss)},
            {Shape::BaleTwine, matte(twineColor, baleSoftness, baleGloss)}};
}

void addBale(Scenery& scenery, std::mt19937& random, Vec2 at)
{
    auto heading = randomBetween(random, 0.f, twoPi);
    auto model = lyingDrum(at, 0.f, heading, 0.f, baleRadius, baleLength);

    for (const auto& part: baleParts())
        scenery.batch.add(part.shape, makeInstance(model, part.material));

    scenery.colliders.add({at, baleColliderRadius, baleRadius * 2.f});
}

Mover makeRollingBale(Vec2 from, Vec2 to, float period, float phase)
{
    auto mover = Mover {};
    mover.from = from;
    mover.to = to;
    mover.period = period;
    mover.phase = phase;
    mover.radius = rollingBaleRadius;
    mover.parts = baleParts();
    mover.collider = {
        mover.positionAt(0.f), rollingBaleRadius, rollingBaleRadius * 2.f};
    return mover;
}
} // namespace Cows
