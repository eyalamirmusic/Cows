#include "Ending.h"

#include <algorithm>

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

Mat4 titlePlacement(const Game& game)
{
    auto rise = titleRise(game.sinceFound);
    auto lift = Vec3 {0.f, -titleDrop * (1.f - rise), 0.f};

    return game.stage() * Mat4::translation(titleCenter + lift)
           * Mat4::rotationX(-0.08f) * Mat4::scale(titleScale * rise);
}

bool loops(const Game& game)
{
    return game.state == Game::State::Found && game.sinceFound > loopTime;
}
} // namespace Cows::Ending
