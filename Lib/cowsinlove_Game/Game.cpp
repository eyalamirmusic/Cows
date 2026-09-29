#include "Game.h"
#include "Animation/Choreography.h"
#include "Cow/Moo.h"

#include <algorithm>
#include <cmath>
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
constexpr auto mooCooldown = 3.2f;
constexpr auto hintTime = 4.f;
constexpr auto fellTime = 2.f;

constexpr auto searchingText =
    "wasd / hjkl / arrows to walk  -  space to jump  -  m to "
    "moo  -  drag to look";
constexpr auto foundText = "you found her";
constexpr auto quitText = "  -  q to quit";
constexpr auto searchingTouchText = "find her  -  drag to look  -  moo for a hint";
constexpr auto fellText = "back on your feet  -  mind the edge";

float headingToward(Vec2 direction)
{
    return std::atan2(-direction.y, direction.x);
}

Vec2 ground(Vec3 point)
{
    return {point.x, point.z};
}

} // namespace

void Game::reset(std::uint32_t newSeed)
{
    seed = newSeed;
    level = makeLevel(seed);

    auto random = std::mt19937 {seed};

    state = State::Searching;
    player = level.start;
    playerHeading = level.startHeading;
    checkpoint = player;
    checkpointHeading = playerHeading;
    sinceFell = 100.f;
    seconds = 0.f;
    level.update(seconds);
    verticalSpeed = 0.f;
    grounded = true;
    bounce = 0.f;
    sinceFound = 0.f;
    sinceMoo = 100.f;
    partner = level.hideout;
    partnerHeading =
        twoPi * std::uniform_real_distribution<float> {0.f, 1.f}(random);
}

void Game::update(float delta, float ahead, float turn, bool jump)
{
    sinceMoo += delta;
    sinceFell += delta;
    seconds += delta;
    level.update(seconds);

    if (state == State::Found)
    {
        sinceFound += delta;
        return;
    }

    auto moving = ahead != 0.f || turn != 0.f;
    playerHeading += turn * turnSpeed * delta;

    if (ahead != 0.f || !level.movers.empty())
    {
        auto direction = Vec2 {std::cos(playerHeading), -std::sin(playerHeading)};
        auto speed = walkSpeed * (ahead > 0.f ? ahead : ahead * backSpeed);
        auto next = ground(player) + direction * (speed * delta);
        next.x = std::clamp(next.x, -playReach, playReach);
        next.y = std::clamp(next.y, -playReach, playReach);
        next = level.pushedOut(next, playerRadius, player.y);
        player.x = next.x;
        player.z = next.y;
    }

    if (jump && grounded)
        verticalSpeed = jumpSpeed;

    verticalSpeed -= gravity * delta;
    player.y += verticalSpeed * delta;

    auto floor = level.floorAt(ground(player), player.y);
    grounded = player.y <= floor;

    if (grounded)
    {
        player.y = floor;
        verticalSpeed = 0.f;

        if (!level.overGap(ground(player))
            && !level.onMover(ground(player), player.y))
        {
            checkpoint = player;
            checkpointHeading = playerHeading;
        }
    }

    if (level.killDepth < 0.f && player.y < level.killDepth)
    {
        player = checkpoint;
        playerHeading = checkpointHeading;
        verticalSpeed = 0.f;
        grounded = true;
        sinceFell = 0.f;
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

void Game::moo()
{
    if (state == State::Searching && sinceMoo > mooCooldown)
        sinceMoo = 0.f;
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

bool Game::hintShowing() const
{
    return state == State::Searching && sinceMoo >= mooAnswerDelay
           && sinceMoo < mooAnswerDelay + hintTime;
}

bool Game::justFell() const
{
    return state == State::Searching && sinceFell < fellTime;
}

std::string footerText(const Game& game,
                       const std::string& hint,
                       bool touchHints,
                       bool quitHint)
{
    auto withQuit = [quitHint](std::string text)
    { return quitHint ? text + quitText : text; };

    if (game.state == Game::State::Found)
        return touchHints ? foundText : withQuit(foundText);

    if (game.hintShowing())
        return hint;

    if (game.justFell())
        return fellText;

    return touchHints ? searchingTouchText : withQuit(searchingText);
}
} // namespace Cows
