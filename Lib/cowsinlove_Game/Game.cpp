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

constexpr auto searchingKeysText =
    "wasd / hjkl / arrows to walk  -  space to jump  -  m to "
    "moo  -  drag to look  -  q to quit";
constexpr auto foundKeysText = "you found her  -  q to quit";
constexpr auto searchingTouchText = "find her  -  drag to look  -  moo for a hint";
constexpr auto foundTouchText = "you found her";
constexpr auto fellText = "back on your feet  -  mind the edge";
constexpr auto menuKeysText = "enter to start  -  q to quit";
constexpr auto editorKeysText = "arrows to choose  -  esc for the menu";

struct PadLabels final
{
    std::string jump;
    std::string moo;
    std::string back;
};

PadLabels padLabels(Hints hints)
{
    switch (hints)
    {
        case Hints::PlayStation:
            return {"cross", "square", "circle"};
        case Hints::Nintendo:
            return {"B", "Y", "A"};
        default:
            break;
    }

    return {"A", "X", "B"};
}

std::string searchingPadText(const PadLabels& labels)
{
    return "left stick to walk  -  " + labels.jump + " to jump  -  " + labels.moo
           + " to moo  -  right stick to look";
}

std::string menuText(Hints hints)
{
    if (isGamepad(hints))
        return padLabels(hints).jump + " to start";

    return hints == Hints::Touch ? "" : menuKeysText;
}

std::string foundPadText(const PadLabels& labels)
{
    return "you found her  -  " + labels.jump + " for another meadow";
}

std::string searchingText(Hints hints)
{
    if (isGamepad(hints))
        return searchingPadText(padLabels(hints));

    return hints == Hints::Touch ? searchingTouchText : searchingKeysText;
}

std::string foundText(Hints hints)
{
    if (isGamepad(hints))
        return foundPadText(padLabels(hints));

    return hints == Hints::Touch ? foundTouchText : foundKeysText;
}

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

    if (state != State::Menu)
        state = State::Searching;

    behindMenu = State::Searching;
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

void Game::start()
{
    if (state == State::Menu)
        state = behindMenu;
}

void Game::openMenu()
{
    if (state == State::Menu)
        return;

    behindMenu = state;
    state = State::Menu;
}

Game::State Game::playing() const
{
    return state == State::Menu ? behindMenu : state;
}

void Game::update(float delta, float ahead, float turn, bool jump)
{
    if (state == State::Menu)
    {
        animate(delta, false);
        return;
    }

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

    animate(delta, moving && grounded);

    if (distance() > foundDistance || std::abs(player.y - partner.y) > foundClimb)
        return;

    auto between = ground(partner) - ground(player);
    state = State::Found;
    stageCenter = partner;
    stageHeading = headingToward(normalize(between));
    sinceFound = 0.f;
}

void Game::animate(float delta, bool walking)
{
    bounce += ((walking ? 1.f : 0.f) - bounce) * std::min(1.f, delta * bounceRate);
    hopClock += delta;

    auto period = coldBeat + (warmBeat - coldBeat) * warmth();
    beatClock += delta * Choreography::heartbeatPeriod / period;
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

EscapeAction escapeAction(Game::State state, bool dressing, bool swinging)
{
    if (swinging)
        return EscapeAction::Swallow;

    if (dressing)
        return EscapeAction::CloseEditor;

    if (state == Game::State::Menu)
        return EscapeAction::PassOn;

    return EscapeAction::OpenMenu;
}

std::string editorText(Hints hints)
{
    if (isGamepad(hints))
        return "left stick to choose  -  " + padLabels(hints).back + " for the menu";

    return hints == Hints::Touch ? "" : editorKeysText;
}

std::string footerText(const Game& game, const std::string& hint, Hints hints)
{
    if (game.state == Game::State::Menu)
        return menuText(hints);

    if (game.state == Game::State::Found)
        return foundText(hints);

    if (game.hintShowing())
        return hint;

    if (game.justFell())
        return fellText;

    return searchingText(hints);
}
} // namespace Cows
