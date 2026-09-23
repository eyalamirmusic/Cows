#include "Game.h"
#include "Choreography.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <random>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto playReach = 90.f;
constexpr auto walkSpeed = 7.f;
constexpr auto turnSpeed = 2.4f;
constexpr auto backSpeed = 0.5f;
constexpr auto bounceRate = 8.f;
constexpr auto playerRadius = 1.f;
constexpr auto foundDistance = 4.5f;
constexpr auto foundClimb = 1.5f;
constexpr auto gravity = 30.f;
constexpr auto jumpSpeed = 12.f;
constexpr auto coldDistance = 80.f;
constexpr auto warmDistance = 5.f;
constexpr auto coldBeat = 0.8f;
constexpr auto warmBeat = 0.25f;
constexpr auto endingLead = 1.f;

float headingToward(Vec2 direction)
{
    return std::atan2(-direction.y, direction.x);
}

Vec2 ground(Vec3 point)
{
    return {point.x, point.z};
}

} // namespace

std::uint32_t clockSeed()
{
    return (std::uint32_t) std::chrono::steady_clock::now()
        .time_since_epoch()
        .count();
}

Game::Game()
{
    auto* fixed = std::getenv("COWS_SEED");
    reset(fixed != nullptr ? (std::uint32_t) std::strtoul(fixed, nullptr, 10)
                           : clockSeed());
}

void Game::reset(std::uint32_t newSeed)
{
    seed = newSeed;
    obstacles = Obstacles {seed};

    auto random = std::mt19937 {seed};

    state = State::Searching;
    player = {};
    playerHeading = 0.f;
    verticalSpeed = 0.f;
    grounded = true;
    bounce = 0.f;
    sinceFound = 0.f;
    partner = obstacles.hideout;
    partnerHeading =
        twoPi * std::uniform_real_distribution<float> {0.f, 1.f}(random);
}

void Game::update(float delta, float ahead, float turn, bool jump)
{
    if (state == State::Found)
    {
        sinceFound += delta;
        return;
    }

    auto moving = ahead != 0.f || turn != 0.f;
    playerHeading += turn * turnSpeed * delta;

    if (ahead != 0.f)
    {
        auto direction = Vec2 {std::cos(playerHeading), -std::sin(playerHeading)};
        auto speed = walkSpeed * (ahead > 0.f ? ahead : ahead * backSpeed);
        auto next = ground(player) + direction * (speed * delta);
        next.x = std::clamp(next.x, -playReach, playReach);
        next.y = std::clamp(next.y, -playReach, playReach);
        next = obstacles.pushedOut(next, playerRadius, player.y);
        player.x = next.x;
        player.z = next.y;
    }

    if (jump && grounded)
        verticalSpeed = jumpSpeed;

    verticalSpeed -= gravity * delta;
    player.y += verticalSpeed * delta;

    auto floor = obstacles.floorAt(ground(player), player.y);
    grounded = player.y <= floor;

    if (grounded)
    {
        player.y = floor;
        verticalSpeed = 0.f;
    }

    bounce += ((moving && grounded ? 1.f : 0.f) - bounce)
              * std::min(1.f, delta * bounceRate);
    hopClock += delta;

    auto period = coldBeat + (warmBeat - coldBeat) * warmth();
    beatClock += delta * Choreography::heartbeatPeriod / period;

    if (distance() > foundDistance || std::abs(player.y - partner.y) > foundClimb)
        return;

    auto between = ground(partner) - ground(player);
    state = State::Found;
    stageCenter = partner;
    stageHeading = headingToward(normalize(between));
    sinceFound = 0.f;
}

float Game::distance() const
{
    return Maths::distance(ground(player), ground(partner));
}

float Game::warmth() const
{
    return std::clamp(
        (coldDistance - distance()) / (coldDistance - warmDistance), 0.f, 1.f);
}

float Game::endingSeconds() const
{
    return Choreography::kissTime(0) - endingLead + sinceFound;
}

Mat4 Game::stage() const
{
    return Mat4::translation(stageCenter) * Mat4::rotationY(stageHeading);
}
} // namespace Cows
