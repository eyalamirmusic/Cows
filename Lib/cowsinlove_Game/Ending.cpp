#include "Ending.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows::Ending
{
namespace
{
float easeInOut(float amount)
{
    auto t = std::clamp(amount, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}
} // namespace

float titleRise(float sinceFound)
{
    return std::max(easeInOut((sinceFound - titleDelay) / titleRiseTime), 0.01f);
}

Vec3 foundStart(const Game& game)
{
    auto heading = game.level.hideoutHeading.value_or(foundStartHeading);
    auto along = Vec3 {std::cos(heading), 0.f, -std::sin(heading)};
    return game.partner - along * foundStartGap;
}

Staging stagingFor(Level::Hiding hiding)
{
    if (hiding == Level::Hiding::Loft)
        return {loftTitleCenter,
                loftTitleScale,
                loftTitleDrop,
                loftCameraHeight,
                loftCameraDistance,
                loftCameraPitch};

    return {titleCenter,
            titleScale,
            titleDrop,
            endingHeight,
            endingDistance,
            endingPitch};
}

Mat4 titlePlacement(const Game& game)
{
    auto staging = stagingFor(game.level.hiding);
    auto rise = titleRise(game.sinceFound);
    auto lift = Vec3 {0.f, -staging.titleDrop * (1.f - rise), 0.f};

    return game.stage() * Mat4::translation(staging.titleCenter + lift)
           * Mat4::rotationX(-0.08f) * Mat4::scale(staging.titleScale * rise);
}

bool loops(const Game& game)
{
    return game.state == Game::State::Found && game.sinceFound > loopTime;
}
} // namespace Cows::Ending
