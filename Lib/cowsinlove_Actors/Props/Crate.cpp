#include "Props/Crate.h"
#include "Props/Props.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t crateColor = 0xb08850;
} // namespace

Vec3 addCrate(Scenery& scenery, std::mt19937& random, Vec2 at)
{
    auto wood = matte(crateColor, 0.1f, 0.1f);
    addBlock(scenery, {at, {0.9f, 0.9f}, 1.5f, {}}, wood);

    if (randomUnit(random) < 0.6f)
    {
        auto stacked = at + Vec2 {1.8f, 0.f};
        addBlock(scenery, {stacked, {0.9f, 0.9f}, 3.f, {}}, wood);
        return {stacked.x, 3.f, stacked.y};
    }

    return {at.x, 1.5f, at.y};
}
} // namespace Cows
