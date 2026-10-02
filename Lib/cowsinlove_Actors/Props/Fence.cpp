#include "Props/Fence.h"
#include "Props/Props.h"

#include <array>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t woodColor = 0xb89a6a;
constexpr auto postSpacing = 2.f;
constexpr auto postSize = 0.2f;
constexpr auto railHeights = std::array {0.55f, 1.15f};
constexpr auto railSize = 0.14f;
} // namespace

void addFence(Scenery& scenery, Vec2 at, float length)
{
    auto wood = matte(woodColor, 0.1f, 0.1f);
    scenery.blocks.add({at, {length * 0.5f, 0.15f}, fenceHeight, {}});

    auto posts = std::max(1, (int) std::round(length / postSpacing));

    for (auto post = 0; post <= posts; ++post)
    {
        auto x = at.x - length * 0.5f + length * (float) post / (float) posts;
        scenery.batch.add(
            Shape::Box,
            makeInstance(Mat4::translation({x, 0.f, at.y})
                             * Mat4::scale({postSize, fenceHeight, postSize}),
                         wood));
    }

    for (auto height: railHeights)
        scenery.batch.add(
            Shape::Box,
            makeInstance(Mat4::translation({at.x, height, at.y})
                             * Mat4::scale({length, railSize * 1.6f, railSize}),
                         wood));
}
} // namespace Cows
